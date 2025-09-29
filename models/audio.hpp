#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/sscm.hpp"
#include "../prt/ols.hpp"
#include <cmath>

//////////////////////////// wavModel /////////////////////////////////

///////////////// Least Mean Squares predictor /////////////////

template <typename F, typename T>
class LMS {
private:
  F *weights, *eg, *buffer;
  F rates[2];
  F rho, complement, eps, prediction;
  int S, D;
public:
  LMS(const int S, const int D, const F lRate, const F rRate, const F rho = 0.95, const F eps = 1e-3) : rates{ lRate, rRate }, rho(rho), complement(1. - rho), eps(eps), prediction(0.), S(S), D(D) {
    assert(S>0 && D>0);
    weights = new F[S+D], eg = new F[S+D], buffer = new F[S+D];
    Reset();
  }
  ~LMS() {
    delete weights, delete eg, delete buffer;
  }
  F Predict(const T sample)
  {
    memmove(&buffer[S+1], &buffer[S], (D-1) * sizeof(F));
    buffer[S] = sample;
    prediction = 0.;
    for (int i=0; i<S+D; i++)
      prediction+= weights[i] * buffer[i];
    return prediction;
  }
  inline float rsqrt(const float x) {
     float r = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
     return (0.5f * (r + 1.0f/(x * r)));
  }
  void Update(const T sample)
  {
    const F error = sample - prediction;
    int i=0;
    for (; i<S; i++) {
      const F gradient = error * buffer[i];
      eg[i] = rho * eg[i] + complement * (gradient * gradient);
      weights[i]+= (rates[0] * gradient * rsqrt(eg[i] + eps));
    }
    for (; i<S+D; i++) {
      const F gradient = error * buffer[i];
      eg[i] = rho * eg[i] + complement * (gradient * gradient);
      weights[i]+= (rates[1] * gradient * rsqrt(eg[i] + eps));
    }
    memmove(&buffer[1], &buffer[0], (S-1) * sizeof(F));
    buffer[0] = sample;
  }
  void Reset() {
    for (int i=0; i<S+D; i++)
      weights[i] = eg[i] = buffer[i] = 0.;
  }
};
//////////////////////////// wavModel /////////////////////////////////

inline U32 SQR(U32 x) {
  return x*x;
}
class wavModel1: public Model {
  BlockData& x;
  Buf& buf;
  private:
  class audio8{
BlockData& x;
   Buf& buf;
   const int nOLS, nLMS, nLnrPrd;
   //nLnrPrd=14
   SmallStationaryContextMap sMap1b[14][3]{
    {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}},
    {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}},
    {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}},
    {{11,1}, {11,1}, {11,1}}, {{11,1}, {11,1}, {11,1}}
  };
  //nOLS=8
  OLS<double, int8_t> ols[8][2]{
    {{128, 24, 0.9975}, {128, 24, 0.9975}},
    {{90, 30, 0.9965}, {90, 30, 0.9965}},
    {{90, 31, 0.996}, {90, 31, 0.996}},
    {{90, 32, 0.995}, {90, 32, 0.995}},
    {{90, 33, 0.995}, {90, 33, 0.995}},
    {{90, 34, 0.9985}, {90, 34, 0.9985}},
    {{28, 4, 0.98}, {28, 4, 0.98}},
    {{28, 3, 0.992}, {28, 3, 0.992}}
  };
  //nLMS=3
  LMS<float, int8_t> lms[3][2]{
    {{1280, 640, 3e-5, 2e-5}, {1280, 640, 3e-5, 2e-5}},
    {{640, 64, 8e-5, 1e-5}, {640, 64, 8e-5, 1e-5}},
    {{2450, 8, 1.6e-5, 1e-6}, {2450, 8, 1.6e-5, 1e-6}}
  };
  //nLnrPrd=14
  int prd[14][2][2];
  int residuals[14][2];
    int stereo, ch;
    U32 mask, errLog, mxCtx;
    int S;
  int wmode;
    public:
   audio8(BlockData& bd):x(bd),buf(bd.buf),nOLS(8), nLMS(3), nLnrPrd(nOLS+nLMS+3),prd{ 0 },residuals{ 0 },stereo(0), ch(0),mask(0), errLog(0), mxCtx(0),S(0),wmode(0){
      
    }   
    
inline int s2(int i) { return int(short(buf(i)+256*buf(i-1))); }
inline int t2(int i) { return int(short(buf(i-1)+256*buf(i))); }

inline int X1(int i) {
  switch (wmode) {
    case 0: return buf(i)-128;
    case 1: return buf(i<<1)-128;
    case 2: return s2(i<<1);
    case 3: return s2(i<<2);
    case 4: return (buf(i)^128)-128;
    case 5: return (buf(i<<1)^128)-128;
    case 6: return t2(i<<1);
    case 7: return t2(i<<2);
    default: return 0;
  }
}

inline int X2(int i) {
  switch (wmode) {
    case 0: return buf(i+S)-128;
    case 1: return buf((i<<1)-1)-128;
    case 2: return s2((i+S)<<1);
    case 3: return s2((i<<2)-2);
    case 4: return (buf(i+S)^128)-128;
    case 5: return (buf((i<<1)-1)^128)-128;
    case 6: return t2((i+S)<<1);
    case 7: return t2((i<<2)-2);
    default: return 0;
  }
}

inline int signedClip8(const int i) {
  return max(-128, min(127, i));
}

void audio8bModel(Mixer& m, int info) {
  const int8_t B = x.c0<<(8-x.bpos);
   if (x.blpos==0 && x.bpos==1) {
  //if (x.bpos==1 &&x.blpos==0) {
      assert((info&2)==0);
      stereo = (info&1);
      mask = 0;
      wmode=info;
      for (int i=0; i<nLMS; i++)
        lms[i][0].Reset(), lms[i][1].Reset();
    }
  if (x.bpos==0) {
    ch=(stereo)?x.blpos&1:0;
    const int8_t s = int(((wmode&4)>0)?buf(1)^128:buf(1))-128;
    const int pCh = ch^stereo;
    int i = 0;
    for (errLog=0; i<nOLS; i++) {
      ols[i][pCh].Update(s);
      residuals[i][pCh] = s-prd[i][pCh][0];
      const U32 absResidual = (U32)abs(residuals[i][pCh]);
      mask+=mask+(absResidual>4);
      errLog+=SQR(absResidual);
    }
    for (int j=0; j<nLMS; j++)
      lms[j][pCh].Update(s);
    for (; i<nLnrPrd; i++)
      residuals[i][pCh] = s-prd[i][pCh][0];
    errLog = min(0xF, ilog2(errLog));
    mxCtx = ilog2(min(0x1F, BitCount(mask)))*2+ch;

    int k1=90, k2=k1-12*stereo;
    for (int j=(i=1); j<=k1; j++, i+=1<<((j>8)+(j>16)+(j>64))) ols[1][ch].Add(X1(i));
    for (int j=(i=1); j<=k2; j++, i+=1<<((j>5)+(j>10)+(j>17)+(j>26)+(j>37))) ols[2][ch].Add(X1(i));
    for (int j=(i=1); j<=k2; j++, i+=1<<((j>3)+(j>7)+(j>14)+(j>20)+(j>33)+(j>49))) ols[3][ch].Add(X1(i));
    for (int j=(i=1); j<=k2; j++, i+=1+(j>4)+(j>8)) ols[4][ch].Add(X1(i));
    for (int j=(i=1); j<=k1; j++, i+=2+((j>3)+(j>9)+(j>19)+(j>36)+(j>61))) ols[5][ch].Add(X1(i));
    if (stereo) {
      for (i=1; i<=k1-k2; i++) {
        const double s = (double)X2(i);
        ols[2][ch].AddFloat(s);
        ols[3][ch].AddFloat(s);
        ols[4][ch].AddFloat(s);
      }
    }
    k1=28, k2=k1-6*stereo;
    for (i=1; i<=k2; i++) {
      const double s = (double)X1(i);
      ols[0][ch].AddFloat(s);
      ols[6][ch].AddFloat(s);
      ols[7][ch].AddFloat(s);
    }
    for (; i<=96; i++) ols[0][ch].Add(X1(i));
    if (stereo) {
      for (i=1; i<=k1-k2; i++) {
        const double s = (double)X2(i);
        ols[0][ch].AddFloat(s);
        ols[6][ch].AddFloat(s);
        ols[7][ch].AddFloat(s);
      }
      for (; i<=32; i++) ols[0][ch].Add(X2(i));
    }
    else
      for (; i<=128; i++) ols[0][ch].Add(X1(i));

    for (i=0; i<nOLS; i++)
      prd[i][ch][0] = signedClip8((int)floor(ols[i][ch].Predict()));
    for (; i<nOLS+nLMS; i++)
      prd[i][ch][0] = signedClip8((int)floor(lms[i-nOLS][ch].Predict(s)));
    prd[i++][ch][0] = signedClip8(X1(1)*2-X1(2));
    prd[i++][ch][0] = signedClip8(X1(1)*3-X1(2)*3+X1(3));
    prd[i  ][ch][0] = signedClip8(X1(1)*4-X1(2)*6+X1(3)*4-X1(4));
    for (i=0; i<nLnrPrd; i++)
      prd[i][ch][1] = signedClip8(prd[i][ch][0]+residuals[i][pCh]);
  }
  for (int i=0; i<nLnrPrd; i++) {
    const U32 ctx = (prd[i][ch][0]-B)*8+x.bpos;
    sMap1b[i][0].set(ctx);
    sMap1b[i][1].set(ctx);
    sMap1b[i][2].set((prd[i][ch][1]-B)*8+x.bpos);
    sMap1b[i][0].mix(m, 6, 1, 2+(i>=nOLS));
    sMap1b[i][1].mix(m, 9, 1, 2+(i>=nOLS));
    sMap1b[i][2].mix(m, 7, 1, 3);
  }
  m.set((errLog<<8)|x.c0, 4096);
  m.set((U8(mask)<<3)|(ch<<2)|(x.bpos>>1), 2048);
  m.set((mxCtx<<7)|(buf(1)>>1), 1280);
  m.set((errLog<<4)|(ch<<3)|x.bpos, 256);
  m.set(mxCtx, 10);
}
};

//16 bit audio model
class audio16{
BlockData& x;
   Buf& buf;
   const int nOLS, nLMS, nLnrPrd;
   //nLnrPrd=14
   SmallStationaryContextMap sMap1b[14][4]{
    {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}},
    {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}},
    {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}},
    {{17,1},{17,1},{17,1},{17,1}}, {{17,1},{17,1},{17,1},{17,1}}
  };
  //nOLS=8
  OLS<double, int16_t> ols[8][2]{
    {{128, 24, 0.9975}, {128, 24, 0.9975}},
    {{90, 30, 0.997}, {90, 30, 0.997}},
    {{90, 31, 0.996}, {90, 31, 0.996}},
    {{90, 32, 0.995}, {90, 32, 0.995}},
    {{90, 33, 0.995}, {90, 33, 0.995}},
    {{90, 34, 0.9985}, {90, 34, 0.9985}},
    {{28, 4, 0.98}, {28, 4, 0.98}},
    {{32, 3, 0.992}, {32, 3, 0.992}}
  };
  //nLMS=3
  LMS<float, int16_t> lms[3][2]{
    {{1280, 640, 5e-5f, 5e-5f}, {1280, 640, 5e-5f, 5e-5f}},
    {{640, 64, 7e-5f, 1e-5f}, {640, 64, 7e-5f, 1e-5f}},
    {{2450, 8, 2e-5f, 2e-6f}, {2450, 8, 2e-5f, 2e-6f}}
  };
  //nLnrPrd=14
  int prd[14][2][2];
  int residuals[14][2];
    int stereo, ch,lsb;
    U32 mask, errLog, mxCtx;
    int S;
  int wmode;
  int16_t sample ;
    public:
   audio16(BlockData& bd):x(bd),buf(bd.buf),nOLS(8), nLMS(3), nLnrPrd(nOLS+nLMS+3),prd{ 0 },residuals{ 0 },stereo(0), ch(0),lsb(0),mask(0), errLog(0), mxCtx(0),S(0),wmode(0),sample(0){
      for (int i=0; i<nLMS; i++)
        lms[i][0].Reset(), lms[i][1].Reset();
    }   
    
inline int s2(int i) { return int(short(buf(i)+256*buf(i-1))); }
inline int t2(int i) { return int(short(buf(i-1)+256*buf(i))); }

inline int X1(int i) {
  switch (wmode) {
    case 0: return buf(i)-128;
    case 1: return buf(i<<1)-128;
    case 2: return s2(i<<1);
    case 3: return s2(i<<2);
    case 4: return (buf(i)^128)-128;
    case 5: return (buf(i<<1)^128)-128;
    case 6: return t2(i<<1);
    case 7: return t2(i<<2);
    default: return 0;
  }
}

inline int X2(int i) {
  switch (wmode) {
    case 0: return buf(i+S)-128;
    case 1: return buf((i<<1)-1)-128;
    case 2: return s2((i+S)<<1);
    case 3: return s2((i<<2)-2);
    case 4: return (buf(i+S)^128)-128;
    case 5: return (buf((i<<1)-1)^128)-128;
    case 6: return t2((i+S)<<1);
    case 7: return t2((i<<2)-2);
    default: return 0;
  }
}

inline int signedClip16(const int i) {
  return max(-32768, min(32767, i));
}
void audio16bModel(Mixer& m, int info) {
  if (x.blpos==0 && x.bpos==1) {
  //if (x.blpos==1 && x.bpos==1) {
     //info|=4;  // comment this line if skipping the endianness transform
     assert((info&2)!=0);
      stereo = (info&1);
      lsb = (info<4);
      mask = 0;
      wmode=info;
      for (int i=0; i<nLMS; i++)
        lms[i][0].Reset(), lms[i][1].Reset();
    }
  if (x.bpos==0) {
      ch=(stereo)?(x.blpos&2)>>1:0;
      lsb=(x.blpos&1)^(wmode<4);
      if ((x.blpos&1)==0)
       {
        sample = (wmode<4)?s2(2):t2(2);
        const int pCh = ch^stereo;
        int i = 0;
        for (errLog=0; i<nOLS; i++) {
          ols[i][pCh].Update(sample);
          residuals[i][pCh] = sample-prd[i][pCh][0];
          const U32 absResidual = (U32)abs(residuals[i][pCh]);
          mask+=mask+(absResidual>128);
          errLog+=SQR(absResidual>>6);
        }
        for (int j=0; j<nLMS; j++){
          lms[j][pCh].Update(sample); }
        for (; i<nLnrPrd; i++){
          residuals[i][pCh] = sample-prd[i][pCh][0]; }
        errLog = min(0xF, ilog2(errLog));

        if (stereo) {
          for (int i=1; i<=24; i++) ols[0][ch].Add(X2(i));
          for (int i=1; i<=104; i++) ols[0][ch].Add(X1(i));
        }
        else
          for (int i=1; i<=128; i++) ols[0][ch].Add(X1(i));

        int k1=90, k2=k1-12*stereo;
        for (int j=(i=1); j<=k1; j++, i+=1<<((j>16)+(j>32)+(j>64))) ols[1][ch].Add(X1(i));
        for (int j=(i=1); j<=k2; j++, i+=1<<((j>5)+(j>10)+(j>17)+(j>26)+(j>37))) ols[2][ch].Add(X1(i));       
        for (int j=(i=1); j<=k2; j++, i+=1<<((j>3)+(j>7)+(j>14)+(j>20)+(j>33)+(j>49))) ols[3][ch].Add(X1(i));
        for (int j=(i=1); j<=k2; j++, i+=1+(j>4)+(j>8)) ols[4][ch].Add(X1(i));
        for (int j=(i=1); j<=k1; j++, i+=2+((j>3)+(j>9)+(j>19)+(j>36)+(j>61))) ols[5][ch].Add(X1(i));

        if (stereo) {
          for (i=1; i<=k1-k2; i++) {
            const double s = (double)X2(i);
            ols[2][ch].AddFloat(s);
            ols[3][ch].AddFloat(s);
            ols[4][ch].AddFloat(s);
          }
        }

        k1=28, k2=k1-6*stereo;
        for (i=1; i<=k2; i++) ols[6][ch].Add(X1(i));
        for (i=1; i<=k1-k2; i++) ols[6][ch].Add(X2(i));

        k1=32, k2=k1-8*stereo;
        for (i=1; i<=k2; i++) ols[7][ch].Add(X1(i));
        for (i=1; i<=k1-k2; i++) ols[7][ch].Add(X2(i));

        for (i=0; i<nOLS; i++)
          prd[i][ch][0] = signedClip16((int)floor(ols[i][ch].Predict()));
        for (; i<nOLS+nLMS; i++)
          prd[i][ch][0] = signedClip16((int)floor(lms[i-nOLS][ch].Predict(sample)));
        prd[i++][ch][0] = signedClip16(X1(1)*2-X1(2));
        prd[i++][ch][0] = signedClip16(X1(1)*3-X1(2)*3+X1(3));
        prd[i  ][ch][0] = signedClip16(X1(1)*4-X1(2)*6+X1(3)*4-X1(4));
        for (i=0; i<nLnrPrd; i++)
          prd[i][ch][1] = signedClip16(prd[i][ch][0]+residuals[i][pCh]);
      }
      mxCtx = ilog2(min(0x1F, BitCount(mask)))*4+ch*2+lsb;
  }

  const int16_t B = int16_t( (info<4)? (lsb)?U8(x.c0<<(8-x.bpos)):(x.c0<<(16-x.bpos))|buf(1) : (lsb)?(buf(1)<<8)|U8(x.c0<<(8-x.bpos)):x.c0<<(16-x.bpos) );

  for (int i=0; i<nLnrPrd; i++) {
    const U32 ctx0 = U16(prd[i][ch][0]-B);
    const U32 ctx1 = U16(prd[i][ch][1]-B);

    sMap1b[i][0].set( (lsb<<16)|(x.bpos<<13)|(ctx0>>(3<<(!lsb))) );
    sMap1b[i][1].set( (lsb<<16)|(x.bpos<<13)|(ctx0>>((!lsb)+(3<<(!lsb)))) );
    sMap1b[i][2].set( (lsb<<16)|(x.bpos<<13)|(ctx0>>((!lsb)*2+(3<<(!lsb)))) );
    sMap1b[i][3].set( (lsb<<16)|(x.bpos<<13)|(ctx1>>((!lsb)+(3<<(!lsb)))) );

    sMap1b[i][0].mix(m, 7, 1/*, 2+(i>=nOLS)*/);
    sMap1b[i][1].mix(m, 10, 1/*, 2+(i>=nOLS)*/);
    sMap1b[i][2].mix(m, 6, 1/*, 3+(i>=nOLS)*/);
    sMap1b[i][3].mix(m, 6, 1/*, 2+(i>=nOLS)*/);
  }

  m.set((errLog<<9)|(lsb<<8)|x.c0, 8192);
  m.set((U8(mask)<<4)|(ch<<3)|(lsb<<2)|(x.bpos>>1), 4096);
  m.set((mxCtx<<7)|(buf(1)>>1), 2560);
  m.set((errLog<<4)|(ch<<3)|(lsb<<2)|(x.bpos>>1), 256);
  m.set(mxCtx, 20);
}
};


audio8 *a8mode;
audio16 *a16mode;
public:
  wavModel1(BlockData& bd);
int inputs() {
    return (14*4)*2;
}
int nets() {
     return 8192+ 4096+ 2560+ 256+ 20;
} 
  int netcount() {return 5;}
int p(Mixer& m,int info,int val2=0);

  virtual ~wavModel1(){
      delete a8mode;
      delete a16mode;
   }
 
};



