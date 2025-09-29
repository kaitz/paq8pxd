#include "EAPM.hpp"
// Filter the context model with APMs


EAPM::EAPM(BlockData& bd):x(bd),a(0x2000,x), a1(0x10000,x), a2(0x10000,x),
 a3(0x10000,x), a4(0x10000,x), a5(0x10000,x), a6(0x10000,x) {
}

int EAPM::p1(int pr0,int pr, int r){
    if (x.fails&0x00000080) --x.failcount;
    x.fails=x.fails*2;
    x.failz=x.failz*2;
    if (x.y) pr^=4095;
    if (pr>=1820) ++x.fails, ++x.failcount;
    if (pr>= 848) ++x.failz;
    int pv, pu,pz,pt;
    pu=(a.p(pr0, x.c0, 3)+7*pr0+4)>>3, pz=x.failcount+1;
    pz+=tri[(x.fails>>5)&3];
    pz+=trj[(x.fails>>3)&3];
    pz+=trj[(x.fails>>1)&3];
    if (x.fails&1) pz+=8;
    pz=pz/2;      

    pu=a4.p(pu,   ( (x.c0*2)^hash(x.buf(1), (x.x5>>8)&255, (x.x5>>16)&0x80ff))&0xffff,r);
    pv=a2.p(pr0,  ( (x.c0*8)^hash(29,x.failz&2047))&0xffff,1+r);
    pv=a5.p(pv,           hash(x.c0,x.w5&0xfffff)&0xffff,r);
    pt=a3.p(pr0, ( (x.c0*32)^hash(19,     x.x5&0x80ffff))&0xffff,r);
    pz=a6.p(pu,   ( (x.c0*4)^hash(min(9,pz),x.x5&0x80ff))&0xffff,r);
    if (x.fails&255)  pr =(pt*6+pu  +pv*11+pz*14 +16)>>5;
    else              pr =(pt*4+pu*5+pv*12+pz*11 +16)>>5;
    return pr;
}
int EAPM::p2(int pr0,int pr8, int r){

  int pr=a.p(pr0,x.Match.length3<<11 |(x.c0<<3)|(x.Misses&0x7));

  int pr1=a1.p(pr0, x.c0+256*x.buf(1));
  int pr2=a2.p(pr0, (x.c0^hash(x.buf(1), x.buf(2)))&0xffff);
  int pr3=a3.p(pr0, (x.c0^hash(x.buf(1), x.buf(2), x.buf(3)))&0xffff);
  pr0=(pr0+pr1+pr2+pr3+2)>>2;

  pr1=a4.p(pr, x.Match.byte+256*x.buf(1));
  pr2=a5.p(pr, (x.c0^hash(x.buf(1), x.buf(2) ))&0xffff);
  pr3=a6.p(pr, (x.c0^hash(x.buf(1), x.buf(2) , x.buf(3) ))&0xffff);
  pr=(pr+pr1+pr2+pr3+2)>>2;

  pr=(pr+pr0+1)>>1;
  return pr;
}
