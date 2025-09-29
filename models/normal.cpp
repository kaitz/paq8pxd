#include "normal.hpp"

extern U8 level;


  normalModel1::normalModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf), N(10), cm(CMlimit(MEM()*32), N,M_NORMAL,
  CM_RUN1+
  CM_RUN0+
  CM_MAIN1+
  CM_MAIN2+
  CM_MAIN3+
  CM_MAIN4+
  CM_M12+
  CM_M6
  ), StateMaps{ 256, 256*256,256*256,256*256 },
  /*rcm7(CMlimit(MEM()/(level>8?8:4)),bd),*/
  rcm9(CMlimit(MEM()/((level>8?8:4))),bd), rcm10(CMlimit(MEM()/(level>8?4:2)),bd){
 }

  
int normalModel1::p(Mixer& m,int val1,int val2){  
  if (x.bpos==0) {
     // if (val2==-1) return 1;
    int i;
    if (val2==0) cm.set(x.cxt[15]);
    for (i=1; i<=7; ++i)
      cm.set(x.cxt[i]);

    //rcm7.set(x.cxt[7]);
    cm.set(x.cxt[9]);
    rcm9.set(x.cxt[10]);
    
    rcm10.set(x.cxt[12]);
    cm.set(x.cxt[14]);

  }
  
  //rcm7.mix(m);
  rcm9.mix(m);
  rcm10.mix(m);
  m.add((stretch(StateMaps[0].p(x.c0-1,x.y)))>>2);
  m.add((stretch(StateMaps[1].p((x.c0-1)|(buf(1)<<8),x.y)))>>2);
  m.add((stretch(StateMaps[2].p((x.c0-1)|(buf(1)<<8),x.y,64)))>>2);
  m.add((stretch(StateMaps[3].p((x.c0-1)|(buf(2)<<8),x.y,64)))>>2);
  return max(0, cm.mix(m)-(N-7)); 
}
  
