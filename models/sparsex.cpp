#include "sparsex.hpp"

 
  sparseModelx::sparseModelx(BlockData& bd): N(31),cm(CMlimit(MEM()*4), N,M_SPARSE),scm1(8,8), scm2(8,8), scm3(8,8),
     scm4(8,8), scm5(8,8),scm6(8,8), scma(8,8),x(bd),buf(bd.buf) {
    }
  int sparseModelx::p(Mixer& m, int seenbefore, int howmany){
  if (x.bpos==0) {
    scm5.set(seenbefore);
    scm6.set(howmany);
    int j=x.tmask;
    U32 h=x.x4<<6;
    U32 d=x.c4&0xffff;
    cm.set(hash(j++,buf(1)+(h&0xffffff00)));
    cm.set(hash(j++,buf(1)+(h&0x00ffff00)));
    cm.set(hash(j++,buf(1)+(h&0x0000ff00)));
    
    h<<=6;
    cm.set(hash(j++,d+(h&0xffff0000)));
    cm.set(hash(j++,d+(h&0x00ff0000)));
    h<<=6, d=x.c4&0xffffff;
    cm.set(hash(j++,d+(h&0xff000000)));
    for (int i=1; i<5; ++i) { 
      cm.set(hash(j++,U32(seenbefore|buf(i)<<8)));
      cm.set(hash(j++,U32((x.buf(i+3)<<8)|buf(i+1))));
    }
    cm.set(hash(j++,x.spaces&0x7fff));
    cm.set(hash(j++,x.spaces&0xff));
    cm.set(hash(j++,x.words&0x1ffff));
    cm.set(hash(j++,x.f4&0x000fffff));
    cm.set(hash(j++,x.tt&0x00000fff));
    h=x.w4<<6;
    cm.set(hash(j++,buf(1)+(h&0xffffff00)));
    cm.set(hash(j++,buf(1)+(h&0x00ffff00)));
    cm.set(hash(j++,buf(1)+(h&0x0000ff00)));
    d=x.c4&0xffff;
    h<<=6;
    cm.set(hash(j++,d+(h&0xffff0000)));
    cm.set(hash(j++,d+(h&0x00ff0000)));
    h<<=6, d=x.c4&0xffffff;
    cm.set(hash(j++,d+(h&0xff000000)));
    cm.set(hash(j++,x.w4&0xf0f0f0ff));
    cm.set(hash(j++,(x.w4&63)*128+(5<<17)));
    cm.set(hash(j++,(x.f4&0xffff)<<11|x.frstchar));
    cm.set(hash(j++,x.spafdo*8*((x.w4&3)==1)));
    
    scm1.set(x.words&127);
    scm2.set((x.words&12)*16+(x.w4&12)*4+(x.f4&0xf));
    scm3.set(x.w4&15);
    scm4.set(x.spafdo*((x.w4&3)==1));
    scma.set(x.frstchar);
  }
  x.rm1=0;
  cm.mix(m);
  x.rm1=1;
  scm1.mix(m);
  scm2.mix(m);
  scm3.mix(m);
  scm4.mix(m);
  scm5.mix(m);
  scm6.mix(m);
  scma.mix(m);
  return 0;
  }

