#include "contextmap2.hpp"
inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}

   void ContextMap2::Update() {
    U64 mask = Table.size()-1;
    for (U32 i=0; i<index; i++) {
        if (Contexts[i]){  
      if (BitState[i])
        *BitState[i] = nex(*BitState[i], lastBit);

      if (bitPos>1 && ByteHistory[i][0]==0)
        BitState[i] = nullptr;
      else {
        U16 chksum = Contexts[i]>>16;
        switch (bitPos) {
          case 0: {
            BitState[i] = BitState0[i] = Table[(Contexts[i]+bits)&mask].Find(chksum);
            // Update pending bit histories for bits 2-7
            if (BitState0[i][3]==2) {
              const int c = BitState0[i][4]+256;
              U8 *p = Table[(Contexts[i]+(c>>6))&mask].Find(chksum);
              p[0] = 1+((c>>5)&1);
              p[1+((c>>5)&1)] = 1+((c>>4)&1);
              p[3+((c>>4)&3)] = 1+((c>>3)&1);
              p = Table[(Contexts[i]+(c>>3))&mask].Find(chksum);
              p[0] = 1+((c>>2)&1);
              p[1+((c>>2)&1)] = 1+((c>>1)&1);
              p[3+((c>>1)&3)] = 1+(c&1);
              BitState0[i][6] = 0;
            }
            // Update byte history of previous context
            ByteHistory[i][3] = ByteHistory[i][2];
            ByteHistory[i][2] = ByteHistory[i][1];
            if (ByteHistory[i][0]==0)  // new context
              ByteHistory[i][0]=2, ByteHistory[i][1]=lastByte;
            else if (ByteHistory[i][1]!=lastByte)  // different byte in context
              ByteHistory[i][0]=1, ByteHistory[i][1]=lastByte;
            else if (ByteHistory[i][0]<254)  // same byte in context
              ByteHistory[i][0]+=2;
            else if (ByteHistory[i][0]==255) // more than one byte seen, but long run of current byte, reset to single byte seen
              ByteHistory[i][0] = 128;

            ByteHistory[i] = BitState0[i]+3;
            HasHistory[i] = *BitState0[i]>15;
            break;
          }
          case 2: case 5: {
            BitState[i] = BitState0[i] = Table[(Contexts[i]+bits)&mask].Find(chksum);
            break;
          }
          case 1: case 3: case 6: BitState[i] = BitState0[i]+1+lastBit; break;
          case 4: case 7: BitState[i] = BitState0[i]+3+(bits&3); break;
        }
      }
      }
    }
  }

  // Construct using Size bytes of memory for Count contexts
  ContextMap2::ContextMap2(const U64 Size, const U32 Count,int mod,int m) :
   C(Count), Table(Size>>6), BitState(Count), BitState0(Count), ByteHistory(Count),
    Contexts(Count), HasHistory(Count),model(mod),inputCount(__builtin_popcount(m)),param(m) {
    assert(Size>=64 && isPowerOf2(Size));
    assert(sizeof(Bucket)==64);
    if (param &CM_M6) Maps6b = new StateMap*[C];
    Maps8b = new StateMap*[C];
    if (param &CM_M12) Maps12b = new StateMap*[C];
    if (param &CM_MR) MapsRun = new StateMap*[C];
    for (U32 i=0; i<C; i++) {
      if(param &CM_M6) Maps6b[i] = new StateMap((1<<6)+8);
      Maps8b[i] = new StateMap(1<<8);
      if (param&CM_M12) Maps12b[i] = new StateMap((1<<12)+(1<<9));
      if (param &CM_MR) MapsRun[i] = new StateMap((1<<12));
      BitState[i] = BitState0[i] = &Table[i].BitState[0][0];
      ByteHistory[i] = BitState[i]+3;
    }
    index = 0;
    lastByte = lastBit = 0;
    bits = 1;  bitPos = 0;
#ifdef VERBOSE
    printf("Model: %s\n",modelNames[model].c_str());
    printf("  CM total inputs: %d\n",inputCount);
    printf("                   MR R2 R1 R0 M1 M2 M3 M4 M5 M12 M6 E1 E2 E3 E4 E5 \n");
    printf("  CM active inputs  %d  %d  %d  %d  %d  %d  %d  %d  %d   %d  %d  %d  %d  %d  %d  %d\n",
    param&CM_MR?1:0,param&CM_RUN2?1:0,param&CM_RUN1?1:0, param&CM_RUN0?1:0, 
    param&CM_MAIN1?1:0, param&CM_MAIN2?1:0, param&CM_MAIN3?1:0, param&CM_MAIN4?1:0, param&CM_MAIN5?1:0,
    param&CM_M12?1:0, param&CM_M6?1:0,
    param&CM_E1?1:0, param&CM_E2?1:0, param&CM_E3?1:0, param&CM_E4?1:0, param&CM_E5?1:0
    );
#endif
  }
  ContextMap2::~ContextMap2() {
    for (U32 i=0; i<C; i++) {
      if (param &CM_M6) delete Maps6b[i];
      delete Maps8b[i];
      if (param&CM_M12) delete Maps12b[i];
      if (param &CM_MR) delete MapsRun[i];
    }
    if (param &CM_M6) delete[] Maps6b;
    delete[] Maps8b;
    if (param&CM_M12) delete[] Maps12b;
    if (param &CM_MR) delete[] MapsRun;
  }

   void ContextMap2::set(U32 ctx) { // set next whole byte context to ctx
  //assert(index>0 && index<=C); // fail if assert on
  if (index==C) index=0;     // model bypass, FIXME
    if (ctx==0){ 
      Contexts[index]=0; 
    } else{
      ctx = ctx*987654323+index; // permute (don't hash) ctx to spread the distribution
      ctx = ctx<<16|ctx>>16;
      Contexts[index] = ctx*123456791+index;
    }
    index++;
  }

  int ContextMap2::mix(Mixer& m, const int Multiplier , const int Divisor) {
    int result = 0;
    lastBit = m.x.y;
    bitPos = m.x.bpos;
    bits+=bits+lastBit;
    lastByte = bits&0xFF;
    if (bitPos==0)
      bits = 1;

    Update();

    for (U32 i=0; i<index; i++) {
      if (Contexts[i]){  
      // predict from bit context
      int state = (BitState[i])?*BitState[i]:0;
      int p1=0 ;
      if (state){
          if (m.x.count>0x5FFFF) p1=Maps8b[i]->p1(state,m.x.y);else p1=Maps8b[i]->p(state,m.x.y);
      } 
      int n0=nex(state, 2), n1=nex(state, 3), k=-~n1;
      const int bitIsUncertain = int(n0 != 0 && n1 != 0);
      k = (k*64)/(k-~n0);
      n0=-!n0, n1=-!n1;

      // predict from last byte in context
      if ((U32)((ByteHistory[i][1]+256)>>(8-bitPos))==bits){
        int RunStats = ByteHistory[i][0]; // count*2, +1 if 2 different bytes seen
        const int predictedBit = (ByteHistory[i][1] >> (7 - bitPos)) & 1U;
        int sign=(predictedBit)*2-1;  // predicted bit + for 1, - for 0
        int value = ilog(RunStats+1)<<(3-(RunStats&1));
        const int byte1IsUncertain = static_cast<const int>(ByteHistory[i][2] != ByteHistory[i][1]);
        const int bp = (0xFEA4U >> (bitPos << 1U)) & 3U; // {0}->0  {1}->1  {2,3,4}->2  {5,6,7}->3
        if (param&CM_RUN1)      m.add(sign * (min(RunStats>>1, 32) << 5)); // +/- 32..1024
        if (param&CM_RUN0)      m.add(sign*value);
        if (param&CM_RUN2)      m.add(sign*value);
        if (param&CM_MR)        m.add(stretch(MapsRun[i]->p( (RunStats>>1) << 4U | bp << 2U | byte1IsUncertain << 1 | predictedBit,m.x.y)) >> (1 + byte1IsUncertain));
      }
      else if (bitPos>0 && (ByteHistory[i][0]&1)>0) {
        if  (param&CM_RUN1)     m.add(0);
        if  (param&CM_RUN2)     m.add(0);
        if  (param&CM_MR)       m.add(0);
        if ((U32)((ByteHistory[i][2]+256)>>(8-bitPos))==bits)
          {if (param&CM_RUN0)   m.add((((ByteHistory[i][2]>>(7-bitPos))&1)*2-1)*128);}
        else if (HasHistory[i] && (U32)((ByteHistory[i][3]+256)>>(8-bitPos))==bits)
          {if (param&CM_RUN0)   m.add((((ByteHistory[i][3]>>(7-bitPos))&1)*2-1)*128);}
        else
          {if (param&CM_RUN0)   m.add(0);}
      }
      else{
       if (param&CM_RUN2)       m.add(0);
       if (param&CM_RUN1)       m.add(0);
       if (param&CM_RUN0)       m.add(0);
       if (param&CM_MR)         m.add(0);
      }
      if (state == 0) {
            if (param&CM_MAIN1) m.add(0);
            if (param&CM_MAIN2) m.add(0);
            if (param&CM_MAIN3) m.add(0);
            if (param&CM_MAIN4) m.add(0);
            if (param&CM_MAIN5) m.add(0);
            if (param&CM_E1) m.add(0);
            if (param&CM_E2) m.add(0);
            if (param&CM_E3) m.add(0);
            if (param&CM_E4) m.add(0);
            if (param&CM_E5) m.add(0);
      } else {
        const int contextIsYoung = int(state <= 2);
        int st=(stretch(p1)*Multiplier)/Divisor;
        if (param&CM_MAIN1) m.add(st >> contextIsYoung);
        if (param&CM_MAIN2) m.add(((p1-2048)*Multiplier)/(2*Divisor));
        if (param&CM_MAIN3) m.add((bitIsUncertain - 1) & st); // when both counts are nonzero add(0) otherwise add(st)
         int p0=4095-p1;
        if (param&CM_MAIN4) m.add((((p1&n0)-(p0&n1))*Multiplier)/(4*Divisor));
        if (param&CM_MAIN5) m.add(0);//
        if (param&CM_E1) m.add(0);// 
        if (param&CM_E2) m.add(0);
        if (param&CM_E3) m.add(0);
        if (param&CM_E4) m.add(0);
        if (param&CM_E5) m.add(0);
        result++;
      }

      if (HasHistory[i]) {
        state  = (ByteHistory[i][1]>>(7-bitPos))&1;
        state |= ((ByteHistory[i][2]>>(7-bitPos))&1)*2;
        state |= ((ByteHistory[i][3]>>(7-bitPos))&1)*4;
      }
      else
        state = 8;

      if (param&CM_M12)  m.add(stretch(Maps12b[i]->p((state<<9)|(bitPos<<6)|k,m.x.y) )>>2);
      if (param&CM_M6)   m.add(stretch(Maps6b[i]->p((state<<3)|bitPos,m.x.y))>>2);
    
      } else{
        if (param&CM_RUN2)  m.add(0);
        if (param&CM_RUN1)  m.add(0);
        if (param&CM_RUN0)  m.add(0);
        if (param&CM_MAIN1) m.add(0);
        if (param&CM_MAIN2) m.add(0);
        if (param&CM_MAIN3) m.add(0);
        if (param&CM_MAIN4) m.add(0);
        if (param&CM_MAIN5) m.add(0);//
        if (param&CM_M12)   m.add(0);
        if (param&CM_M6)    m.add(0);
        if (param&CM_E1) m.add(0);//
        if (param&CM_E2) m.add(0);//
        if (param&CM_E3) m.add(0);//
        if (param&CM_E4) m.add(0);//
        if (param&CM_E5) m.add(0);//
        if (param&CM_MR) m.add(0);
      }
    }
    if (bitPos==7) index = 0;
    return result;
  }




