#include "APM.hpp"
// An APM maps a probability and a context to a new probability.  Methods:
//
// APM a(n) creates with n contexts using 96*n bytes memory.
// a.pp(y, pr, cx, limit) updates and returns a new probability (0..4095)
//     like with StateMap.  pr (0..4095) is considered part of the context.
//     The output is computed by interpolating pr into 24 ranges nonlinearly
//     with smaller ranges near the ends.  The initial output is pr.
//     y=(0..1) is the last bit.  cx=(0..n-1) is the other context.
//     limit=(0..1023) defaults to 255.


int APM::p(int pr, int cx,const  int y,int limit) {
    assert(pr>=0 && pr<4096);
    assert(cx>=0 && cx<N/steps);
    assert(y==0 || y==1);
    assert(limit>0 && limit<1024);
    update2(y,limit);
    pr=(stretch(pr)+2048)*(steps-1);
    int wt=pr&0xfff;  // interpolation weight of next element
    cx=cx*steps+(pr>>12);
    assert(cx>=0 && cx<N-1);
    cxt=cx+(wt>>11);
    pr=((t[cx]>>13)*(0x1000-wt)+(t[cx+1]>>13)*wt)>>19;
    return pr;
  }


APM::APM(int n,const int s): StateMap(n*s),steps(s) {
  for (int i=0; i<N; ++i) {
    int p=((i%steps*2+1)*4096)/(steps*2)-2048;
    t[i]=(U32(squash(p))<<20)+6;
  }
}
