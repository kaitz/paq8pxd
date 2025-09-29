#include "match1.hpp"
//////////////////////////// matchModel ///////////////////////////
extern U8 level;


  void matchModel1::Update() {
        delta = false;
    if (length==0 && x.Match.bypass)
      x.Match.bypass = false; // can quit bypass mode on byte boundary only
    // update hashes
    for (U32 i=0, minLen=MinLen+(NumHashes-1)*StepSize; i<NumHashes; i++, minLen-=StepSize) {
      hashes[i] = hash(minLen);
      for (U32 j=1; j<=minLen; j++)
        hashes[i] = combine(hashes[i], buffer(j)<<i);
      hashes[i]&=mask;
    }
    // extend current match, if available
    if (length) {
      index++;
      if (length<MaxLen)
        length++;
    }
    // or find a new match, starting with the highest order hash and falling back to lower ones
    else {
      U32 minLen = MinLen+(NumHashes-1)*StepSize, bestLen = 0, bestIndex = 0;
      for (U32 i=0; i<NumHashes && length<minLen; i++, minLen-=StepSize) {
        index = Table[hashes[i]];
        if (index>0) {
          length = 0;
          while (length<(minLen+MaxExtend) && buffer(length+1)==buffer[index-length-1])
            length++;
          if (length>bestLen) {
            bestLen = length;
            bestIndex = index;
          }
        }
      }
      if (bestLen>=minLen) {
        length = bestLen-(MinLen-1); // rebase, a length of 1 actually means a length of MinLen
        index = bestIndex;
      }
      else
        length = index = 0;
    }
    // update position information in hashtable
    for (U32 i=0; i<NumHashes; i++)
      Table[hashes[i]] = x.buf.pos;
      expectedByte = buffer[index];
    iCtx+=x.y, iCtx=(buffer(1)<<8)|expectedByte;
    const uint32_t lengthIlog2 = ilog2(length + 1);
    //no match:      lengthIlog2=0
    //length=1..2:   lengthIlog2=1
    //length=3..6:   lengthIlog2=2
    //length=7..14:  lengthIlog2=3
    //length=15..30: lengthIlog2=4
    const uint8_t length3 = min(lengthIlog2, 3); // 2 bits
  
    SCM[0].set(expectedByte);
    SCM[1].set(expectedByte);
    SCM[2].set(x.buf.pos);
    Maps[0].set((expectedByte<<8)|buffer(1));
    Maps[1].set(hash(expectedByte, x.c0, buffer(1), length3 ));
    Maps[2].set(iCtx());
    x.Match.byte = (length)?expectedByte:0;
  }

  matchModel1::matchModel1(BlockData& bd, U32 val1) :
    x(bd),buffer(bd.buf),Size(level>9?0x80000000: CMlimit(MEM()*4)),
    Table(Size/sizeof(U32)),
    StateMaps{56*256, 8*256*256+1, 256*256 },
    SCM{ {8,8},{11,1},{8,8} },
    Maps{ {16}, {22,1}, {4,1} },
    iCtx{19,1},
    hashes{ 0 },
    ctx{ 0 },
    length(0),
    mask(Size/sizeof(U32)-1),
    expectedByte(0),
    delta(false),
    canBypass(true)
  {
      x.Match.bypass=false;
      x.Match.bypassprediction=2048;
    assert((Size&(Size-1))==0);
  }
  int matchModel1::p(Mixer& m,int val1,int val2) {
    if (x.bpos==0)
      Update();
   else {
      U8 B = x.c0<<(8-x.bpos);
      SCM[1].set((x.bpos<<8)|(expectedByte^B));
      Maps[1].set(hash(expectedByte, x.c0, buffer(1), buffer(2), min(3,(int)ilog2(length+1))));
      iCtx+=x.y, iCtx=(x.bpos<<16)|(buffer(1)<<8)|(expectedByte^B);
      Maps[2].set(iCtx());
    }
    const int expectedBit =length != 0 ? (expectedByte>>(7-x.bpos))&1: 0;;

    if(length>0) {
      const bool isMatch = x.bpos==0 ? buffer(1)==buffer[index-1] : ((expectedByte+256)>>(8-x.bpos))==x.c0; // next bit matches the prediction?
      if(!isMatch) {
        delta = (length+MinLen)>DeltaLen;
        length = 0;
      }
    }

    if (!(canBypass && x.Match.bypass)) {
      for (U32 i=0; i<NumCtxs; i++)
        ctx[i] = 0;
      if (length>0) {
          if (length<=16)
            ctx[0] = (length-1)*2 + expectedBit; // 0..31
          else
            ctx[0] = 24 + (min(length-1, 63)>>2)*2 + expectedBit; // 32..55
          ctx[0] = ((ctx[0]<<8) | x.c0);
          ctx[1] = ((expectedByte<<11) | (x.bpos<<8) | buffer(1)) + 1;
          const int sign = 2*expectedBit-1;
          m.add(sign*(min(length,32)<<5)); // +/- 32..1024
          m.add(sign*(ilog(length)<<2));   // +/-  0..1024
        }
        else { // no match at all or delta mode
          m.add(0);
          m.add(0);
        }

      if (delta)
        ctx[2] = (expectedByte<<8) | x.c0;

      for (U32 i=0; i<NumCtxs; i++) {
        const U32 c = ctx[i];
        const U32 p = StateMaps[i].p(c,x.y);
        if (c!=0)
          m.add((stretch(p)+1)>>1);
        else
          m.add(0);
      }

      SCM[0].mix(m);
      SCM[1].mix(m, 6);
      SCM[2].mix(m, 5);
      Maps[0].mix(m);
      Maps[1].mix(m);
      Maps[2].mix(m);
     x.Match.length3=(min(ilog2(x.Match.length),3));
      m.set(min( x.Match.length3 + 1, 7), 8);
    }
    x.Match.bypassprediction = length==0 ? 2048 : (expectedBit==0 ? 1 : 4095);
    x.Match.length = length;
    return ilog(length);
  }

