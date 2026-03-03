#include "im1bit.hpp"

// This model is from paq8px

//////////////////////////// im1bitModel /////////////////////////////////
// Model for 1-bit image data

im1bitModel1::im1bitModel1( BlockData& bd,U32 val): x(bd),buf(bd.buf),
    r0(0),r1(0),r2(0),r3(0),r4(0),r5(0),r6(0),r7(0),r8(0),
    t(C),N(18), cxt(N),counts(C),mapL({{ 16, 128},{ 16, 128},{ 16, 128},{ 16, 128},{ 16, 128}} )/*,t1(65536/2)*/ {

    sm=new StateMap[N];
    //cp=t1[0]+1;

    // Set model mixer contexts and parameters
    mxp.push_back( {256,64,0,28,&mxcxt[0],0} );
    mxp.push_back( {256,64,0,28,&mxcxt[1],0} );
    mxp.push_back( {256,64,0,28,&mxcxt[2],0} );
    mxp.push_back( {256,64,0,28,&mxcxt[3],0} );
    mxp.push_back( { 16,64,0,28,&mxcxt[4],0} );
    mxp.push_back( { 64,64,0,28,&mxcxt[5],0} );
    mxp.push_back( { 41,64,0,28,&mxcxt[6],0} );
    mxp.push_back( {  8,64,0,28,&mxcxt[7],0} );
    
}
void im1bitModel1::add(Mixers& m, uint32_t n0, uint32_t n1) {
  int p1 = ((n1 + 1) << 12) / ((n0 + n1) + 2);
  m.add(stretch(p1) >> 1);
  //m.add((p1 - 2048) >> 2);
}
int im1bitModel1::p(Mixers& m,int w, int val2) {
    // update the model
    int i;
    for (i=0; i<N; i++) {
        t[cxt[i]]=nex(t[cxt[i]],x.y);
        
        uint32_t c = counts[cxt[i]];
    int cnt0 = (c >> 16) & 65535;
    int cnt1 = (c & 65535);

    if (x.y == 0) cnt0++; else cnt1++;
    if (cnt0 + cnt1 >= 65536) { cnt0 >>= 1; cnt1 >>= 1; } //prevent overflow in add()
      
    c = cnt0 << 16 | cnt1;
    counts[cxt[i]] = c;
    }
    
    //count run
    /*if (cp[0]==0 || cp[1]!=x.y) cp[0]=1, cp[1]=x.y;
    else if (cp[0]<255) ++cp[0];
    cp=t1[x.c4]+1;*/
    // update the contexts (pixels surrounding the predicted one)
    r0+=r0+x.y;
    r1+=r1+((x.buf(w*1-1)>>(7-x.bpos))&1);
    r2+=r2+((x.buf(w*2-1)>>(7-x.bpos))&1);
    r3+=r3+((x.buf(w*3-1)>>(7-x.bpos))&1);
    r4+=r4+((x.buf(w*4-1)>>(7-x.bpos))&1);
    r5+=r5+((x.buf(w*5-1)>>(7-x.bpos))&1);
    r6+=r6+((x.buf(w*6-1)>>(7-x.bpos))&1);
    r7+=r7+((x.buf(w*7-1)>>(7-x.bpos))&1);
    r8+=r8+((x.buf(w*8-1)>>(7-x.bpos))&1);
      int c = 0; //base for each context
      const int y=x.y;
  i = 0; //context index
  
  //   x
  //  x?
  cxt[i++] = c + (y | (r1 >> 8 & 1) << 1);
  c += 1 << 2;

  //  xxx
  //  x?
  uint32_t surrounding4 = y | (r1 >> 7 & 7) << 1;
  cxt[i++] = c + surrounding4;
  c += 1 << 4;

  //    x
  //    x
  //  xx?
  cxt[i++] = c + ((r0 & 3) | (r1 >> 8 & 1) << 2 | ((r2 >> 8) & 1) << 3);
  c += 1 << 4;

  //    x
  //    x
  //    x
  // xxx?
  uint32_t mCtx6 = ((r0 & 7) | (r1 >> 8 & 1) << 3 | (r2 >> 8 & 1) << 4 | (r3 >> 8 & 1) << 5);
  cxt[i++] = c + mCtx6;
  c += 1 << 6;
  
//uint32_t mCtx6 =(r0>>2)&1)|(((r1>>1)&0xF0)>>3)|((r2>>3)&0xA));// ((r0 & 7) | (r1 >> 8 & 1) << 3 | (r2 >> 8 & 1) << 4 | (r3 >> 8 & 1) << 5);
  cxt[i++] = c +( (r0>>2)&1)|(((r1>>1)&0xF0)>>3)|(((r2>>3)&0xA)>>1);
  c += 1 << 7;
  //  xx 
  //   xxx
  // xxx?
  cxt[i++] = c + (r0 & 7) | (r1 >> 7 & 7) << 3 | (r2 >> 9 & 3) << 6;
  c += 1 << 8; 
  
  //   x
  //   x
  //  xxxxx
  //  x?
  cxt[i++] = c + (y | (r1 >> 5 & 0x1f) << 1 | (r2 >> 8 & 1) << 6 | (r3 >> 8 & 1) << 7);
  c += 1 << 8;

  //   xx
  //   xx
  //   xxx
  //  x?
  cxt[i++] = c + (y | (r1 >> 6 & 7) << 1 | (r2 >> 7 & 3) << 4 | (r3 >> 7 & 3) << 6);
  c += 1 << 8;

  //  x
  //  x
  //  x
  //  x
  //  x
  //  x
  //  x
  //  x
  //  ?
  cxt[i++] = c + ((r1 >> 8 & 1) << 7 | (r2 >> 8 & 1) << 6 | (r3 >> 8 & 1) << 5 | (r4 >> 8 & 1) << 4 | (r5 >> 8 & 1) << 3 | (r6 >> 8 & 1) << 2 | (r7 >> 8 & 1) << 1 | (r8 >> 8 & 1));
  c += 1 << 8;

  // xxxxxxxx?
  cxt[i++] = c + (r0 & 0xff);
  c += 1 << 8;

  //  xx
  //  xx
  //  xx
  //  xx
  //  x?
  cxt[i++] = c + (y | (r1 >> 8 & 3) << 1 | (r2 >> 8 & 3) << 3 | (r3 >> 8 & 3) << 5 | (r4 >> 8 & 3) << 7);
  c += 1 << 9;

  //  xxxxxx
  //   xxxx?
  cxt[i++] = c + ((r0 & 0x0f) | (r1 >> 8 & 0x3f) << 4);
  c += 1 << 10;
  
  //       xx
  //   xxxxxxx
  //  xxx?
  cxt[i++] = c + ((r0 & 7) | (r1 >> 4 & 0x7f) << 3 | ((r2 >> 5) & 3) << 10);
  c += 1 << 12;
  
  // xxxxx   
  // xxxxx
  // xx?
  uint32_t surrounding12 = (r0 & 3) | (r1 >> 6 & 0x1f) << 2 | (r2 >> 6 & 0x1f) << 7;
  cxt[i++] = c + surrounding12;
  c += 1 << 12;

  // xxxxxxx   
  // xxxxxxx   
  // xxxxxxx
  // xxx?
  uint32_t surrounding24 = (r0 & 7) | (r1 >> 5 & 0x7f) << 3 | (r2 >> 5 & 0x7f) << 10 | (r3 >> 5 & 0x7f) << 17;
  
  // xxxxxxxxx
  // x.......x
  // x.......x
  // x.......x
  // x...?
  uint32_t frame16 = (r0 >> 3 & 1) | (r1 >> 12 & 1) << 1 | (r2 >> 12 & 1) << 2 | (r3 >> 12 & 1) << 3 | (r4 >> 4 & 0x1ff) << 4 | (r3 >> 4 & 1) << 13 | (r2 >> 4 & 1) << 14 | (r1 >> 4 & 1) << 15;
//contexts: surrounding pixel counts (most useful for dithered images)
  int bitcount04 = BitCount(surrounding4); //bitcount for the surrounding 4 pixels
  int bitcount12 = BitCount(surrounding12); //... 12 pixels
  int bitcount24 = BitCount(surrounding24); //... 24 pixels
  int bitcount40 = BitCount(frame16) + bitcount24; //...40 pixels
  
  cxt[i++] = c + bitcount04; 
  c += 5;  

  cxt[i++] = c + bitcount12; 
  c += 13; 

  cxt[i++] = c + bitcount24; 
  c += 25; 

  cxt[i++] = c + bitcount40; 
  c += 41; 

  assert(i == N);
  assert(c == C);
  /*  cxt[0]=(r0&0x7)|(r1>>4&0x38)|(r2>>3&0xc0);
    cxt[1]=0x100+   ((r0&1)|(r1>>4&0x3e)|(r2>>2&0x40)|(r3>>1&0x80));
    cxt[2]=0x200+   ((r0&1)|(r1>>4&0x1d)|(r2>>1&0x60)|(r3&0xC0));
      xxx0x0xx00x0
      xx00x0xx0xx0
      xxxx0x0xx0xx
      xxxxx0x0xxxx
      xxx0000xxxxx
    xxxx0xl
    cxt[3]=0x300+   ((r0&1)|((r0<<1)&4)|((r1>>1)&0xF0)|((r2>>3)&0xA));//
    cxt[4]=0x400+   ((r0>>4&0x2AC)|(r1&0xA4)|(r2&0x349)|(!(r3&0x14D)));
    cxt[5]=0x800+   ((r0&1)|((r1>>4)&0xE)|((r2>>1)&0x70)|((r3<<2)&0x380));//
    cxt[6]=0xC00+   (((r1&0x30)^(r3&0x0c0c))|(r0&3));
    cxt[7]=0x1000+  ((!(r0&0x444))|(r1&0xC0C)|(r2&0xAE3)|(r3&0x51C));
    cxt[8]=0x2000+  ((r0&7)|((r1>>1)&0x3F8)|((r2<<5)&0xC00));//
    cxt[9]=0x3000+  ((r0&0x3f)^(r1&0x3ffe)^(r2<<2&0x7f00)^(r3<<5&0xf800));
    cxt[10]=0x13000+((r0&0x3e)^(r1&0x0c0c)^(r2&0xc800));
*/
//     xxxxx
  //    xxxxxxx
  // xxxxxxxxxxxxxx
  // xxxxxx?
  mapL[0].set(hash((r0 & 0x3f), (r1 >> 1 & 0x1fff), (r2 >> 5 & 0x7f), (r3 >> 6 & 0x1f))); // 6+13+7+5=31 bits

  //         xx
  //         xx
  //   xxxxxxxx
  // xxxxxxxx?
  mapL[1].set(hash((r0 & 0xff), (r1 >> 7 & 0xff), (r2 >> 7 & 3)<<2 | (r3 >> 7 & 3))); // 8+8+2+2=20 bits

  //    xx
  //    xx
  //    xx
  //    xx
  //    xx
  //    xx
  //    xx
  //    xx
  //   x?
  mapL[2].set(hash(y | (r1 >> 7 & 3) << 1, (r2 >> 7 & 3) | (r3 >> 7 & 3) << 2, (r4 >> 7 & 3) | (r5 >> 7 & 3) << 2, (r6 >> 7 & 3) | (r7 >> 7 & 3) << 2, (r8 >> 7 & 3)));  // 8*2+1=17 bits hashed to 16 bits
  
  //  xxxxxxx
  //  xxxxxxx
  //  xxxxxxx
  //  xxx?
  mapL[3].set(hash(surrounding24)); // 24 bits
  
  // xxxxxxxxx
  // xxxxxxxxx
  // xxxxxxxxx
  // xxxxxxxxx
  // xxxx?
  mapL[4].set(hash(surrounding24, frame16)); // 40 bits

  mapL[0].mix(m);
  mapL[1].mix(m);
  mapL[2].mix(m);
  mapL[3].mix(m);
  mapL[4].mix(m);
  
    // predict
    for (i=0; i<N; i++) {
        const int state=t[cxt[i]];
        if (state) {
        m.add(stretch(sm[i].p(state,x.y))>>1);
        uint32_t n0 = (counts[cxt[i]] >> 16) & 65535;
      uint32_t n1 = (counts[cxt[i]]) & 65535;
      add(m, n0, n1);
      }
        else m.add(0), m.add(0);
    }
    //run
   /* if (cp[1]==x.y)
    m.add(((cp[1]&1)*2-1)*ilog(cp[0]+1)*2);
    else
    m.add(0);*/
//for dithered images
  add(m, bitcount04, 04 - bitcount04);
  add(m, bitcount12, 12 - bitcount12);
  add(m, bitcount24, 24 - bitcount24);
  add(m, bitcount40, 40 - bitcount40);
    mxcxt[0]=(r0&0x7)|(r1>>4&0x38)|(r2>>3&0xc0);
    mxcxt[1]=((r1&0x30)^(r3&0x0c))|(r0&3);
    mxcxt[2]=(r0&1)|(r1>>4&0x3e)|(r2>>2&0x40)|(r3>>1&0x80);
    mxcxt[3]=(r0&0x3e)^((r1>>8)&0x0c)^((r2>>8)&0xc8);
    //mxcxt[4]=cp[0];
    mxcxt[4]=surrounding4-1;
    mxcxt[5]=mCtx6;
    mxcxt[6]=bitcount40-1;
    mxcxt[7]=x.bpos;
    return surrounding12;
}
