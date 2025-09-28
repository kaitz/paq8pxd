#include "statemap.hpp"
//////////////////////////// StateMap, APM //////////////////////////

// A StateMap maps a context to a probability.  Methods:
//
// Statemap sm(n) creates a StateMap with n contexts using 4*n bytes memory.
// sm.p(y, cx, limit) converts state cx (0..n-1) to a probability (0..4095).
//     that the next y=1, updating the previous prediction with y (0..1).
//     limit (1..1023, default 1023) is the maximum count for computing a
//     prediction.  Larger values are better for stationary sources.

extern int dt[1024];  // i -> 16K/(i+i+3)

inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}

void StateMap::update(const int y, int limit) {
    assert(cxt>=0 && cxt<N);
    assert(y==0 || y==1);
    U32 *p=&t[cxt], p0=p[0];
#ifdef SM   
    int n=p0&1023, pr=p0>>13;  // count, prediction
    p0+=(n<limit);
    U32 err=(y<<19)-pr;
    p0+=((err))*dt[n]&0xfffffc00;

#else
    int n=p0&1023, pr=p0>>10;  // count, prediction
    p0+=(n<limit);
    p0+=(((y<<22)-pr)>>3)*dt[n]&0xfffffc00;
#endif
    p[0]=p0;
  }
  
void StateMap::update1(const int y, int limit) {
    assert(cxt>=0 && cxt<N);
    assert(y==0 || y==1);
    U32   p0=t[cxt];
#ifdef SM   
    // count, prediction
    p0+=(y<<19)-(p0>>13);
#else
    int pr=p0>>10;  // count, prediction
    p0+=(((y<<22)-pr)>>3);
#endif
    t[cxt]=p0;
  }
void StateMap::update2(const int y, int limit) {
    assert(cxt>=0 && cxt<N);
    assert(y==0 || y==1);
    U32 *p=&t[cxt], p0=p[0];
 
    int n=p0&1023, pr=p0>>10;  // count, prediction
    p0+=(n<limit);
    p0+=(((y<<22)-pr)>>3)*dt[n]&0xfffffc00;
 
    p[0]=p0;
  }
void StateMap::Reset(int Rate){
    for (int i=0; i<N; ++i)
      t[i]=(t[i]&0xfffffc00)|min(Rate, t[i]&0x3FF);
  }
  // update bit y (0..1), predict next bit in context cx
  int StateMap::p(int cx,const int y,int limit) {
    assert(cx>=0 && cx<N);
    assert(limit>0 && limit<1024);
    assert(y==0 || y==1);
    update(y,limit);

    return t[cxt=cx]>>20;
  }
  int StateMap::p1(int cx,const int y,int limit) {
    assert(cx>=0 && cx<N);
    assert(limit>0 && limit<1024);
    assert(y==0 || y==1);
    update1(y,limit);
    return t[cxt=cx]>>20;
  }
  int StateMap::p2(int cx,const int y,int limit) {
    assert(cx>=0 && cx<N);
    assert(limit>0 && limit<1024);
    assert(y==0 || y==1);
    update2(y,limit);
    return t[cxt=cx]>>20;
  }

StateMap::~StateMap() {
/*    if (N==256){
        for (U32 cx=0; cx<255; ++cx) {
            printf ("%d ",t[cx] >> 20);
   }
   printf ("\n" ); }*/
}

StateMap::StateMap(int n): N(n), cxt(0), t(n) {
  for (U32 cx=0; cx<N; ++cx) {
        U8 state=U8(cx&255);
        U32 n0=nex(state,2)*3+1;
        U32 n1=nex(state,3)*3+1;
        t[cx] = ((n1<<20)/(n0+n1))<<12;
  }
}
