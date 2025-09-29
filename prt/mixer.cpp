#include "mixer.hpp"
//////////////////////////// Mixer /////////////////////////////

// Mixer m(N, M, S=1, w=0) combines models using M neural networks with
//   N inputs each, of which up to S may be selected.  If S > 1 then
//   the outputs of these neural networks are combined using another
//   neural network (with parameters S, 1, 1).  If S = 1 then the
//   output is direct.  The weights are initially w (+-32K).
//   It is used as follows:
// m.update() trains the network where the expected output is the
//   last bit (in the global variable y).
// m.add(stretch(p)) inputs prediction from one of N models.  The
//   prediction should be positive to predict a 1 bit, negative for 0,
//   nominally +-256 to +-2K.  The maximum allowed value is +-32K but
//   using such large values may cause overflow if N is large.
// m.set(cxt, range) selects cxt as one of 'range' neural networks to
//   use.  0 <= cxt < range.  Should be called up to S times such
//   that the total of the ranges is <= M.
// m.p() returns the output prediction that the next bit is 1 as a
//   12 bit number (0 to 4095).




inline U32 SQR(U32 x) {
  return x*x;
}

#if defined(__MMX__)
typedef __m128i XMM;
#endif
#if defined(__AVX2__)
typedef __m256i YMM;
#endif
#define DEFAULT_LEARNING_RATE 7


  // Adjust weights to minimize coding cost of last prediction
  void Mixer::update() {
    for (int i=0; i<ncxt; ++i) {
      int err;
     if (ncxt>1)  err=((x.y<<12)-pr[i])*mrate[i]/2;
     else  err=((x.y<<12)-pr[i])*lrate/lshift;
      assert(err>=-32768 && err<32768);
      if(err>=-merr[i] && err<=merr[i]) err=0;
      train(&tx[0], &wx[cxt[i]*N], nx, err);
    }
    reset();
  }
   void Mixer::update1() {
    int target=x.y<<12;
    if(nx>0)
    for (int i=0; i<ncxt; ++i) {
      int err=target-pr[i];
      train(&tx[0], &wx[cxt[i]*N], nx, err*rates[i]);

        U32 logErr=min(0xF,ilog2(abs(err)));
        info[i].Sum-=SQR(info[i].Data[1]>>28);
        info[i].Data[1]<<=4; info[i].Data[1]|=info[i].Data[0]>>28;
        info[i].Data[0]<<=4; info[i].Data[0]|=logErr;
        info[i].Sum+=SQR(logErr);
        info[i].Collected+=info[i].Collected<4096;
        info[i].Mask<<=1; info[i].Mask|=(logErr<=((info[i].Data[0]>>4)&0xF));
        U32 count=BitCount(info[i].Mask);
        if (info[i].Collected>=64 && (info[i].Sum>1500+U32(rates[i])*64 || count<9 || (info[i].Mask&0xFF)==0)){
          rates[i]=DEFAULT_LEARNING_RATE;
          memset(&info[i], 0, sizeof(ErrorInfo));
        }
        else if (info[i].Collected==4096 && info[i].Sum>=56 && info[i].Sum<=144 && count>28-U32(rates[i]) && ((info[i].Mask&0xFF)==0xFF)){
          rates[i]-=rates[i]>2;
          memset(&info[i], 0, sizeof(ErrorInfo));
        }
    }
    reset();
  }

  void Mixer::update2() {
    if (nx==0)         { reset(); return;}
    if (x.filetype==EXE) update1();
    else                 update();
  }
  // Input x (call up to N times)
  void Mixer::add(int x) {
    assert(nx<N);
    tx[nx++]=x;
  }

  void Mixer::sp(int x) {
    int p=tx[nx];
    p=p*x/4;
    tx[nx++]=p;
  }
  // Set a context (call S times, sum of ranges <= M)
  void Mixer::set(int cx, int range,int mr,int me) {
    assert(range>=0);
    assert(ncxt<S);
    assert(cx>=0);
    assert(cx<range);
    assert(base+cx<M);
    mrate[ncxt]=mr;
    merr[ncxt]=me;
    cxt[ncxt++]=base+cx;
    base+=range;
  }
  
  void Mixer::setl(int l, int r) {   
   if (mp) mp->setl(l,r);
   else lrate=l,lshift=r;
  }

  // predict next bit
  int Mixer::p(const int shift0, const int shift1) {
    while (nx&15) tx[nx++]=0;  // pad
    if (mp) {  // combine outputs
      mp->update2();
      for (int i=0; i<ncxt; ++i) {
        int dp=((dot_product(&tx[0], &wx[cxt[i]*N], nx)));
        dp=dp>>(5+shift0);
        pr[i]=squash(dp);
        if (dp<-2047) {
            dp=-2047;
        }
        else if (dp>2047) {
            dp=2047;
        }
        mp->add(dp);
      }
      mp->set(0, 1);
      return mp->p(shift0, shift1);
    }
    else {  // S=1 context
    return pr[0]=squash(dot_product(&tx[0], &wx[0], nx)>>(8+shift1));
    }
  }



Mixer::~Mixer() {
  delete mp;
}

Mixer::Mixer(int n, int m, BlockData& bd, int s, int w, int g,int h):
    N((n+15)&-16), M(m), S(s),tx(N), wx(N*M),
    cxt(S),mrate(S),merr(S), ncxt(0), base(0),nx(0), pr(S), mp(0),x(bd), info(S),
    rates(S),lrate(S>1?7:g),lshift(S>1?1:h){
  assert(n>0 && N>0 && (N&15)==0 && M>0);
   int i;
  for (i=0; i<S; ++i){
    pr[i]=2048; //initial p=0.5
    rates[i] = DEFAULT_LEARNING_RATE;
    mrate[i]=14;
    memset(&info[i], 0, sizeof(ErrorInfo));
  }

  for (i=0; i<N*M; ++i)
    wx[i]=w;
  if (S>1) mp=new Mixer(S, 1,x ,1,0,g,h);
}

