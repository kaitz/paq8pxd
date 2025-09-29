#include "linear.hpp"


  linearPredictionModel::linearPredictionModel(BlockData& bd,U32 val):x(bd),buf(bd.buf),nOLS(3),nLnrPrd(3+2)  {
  }

  int linearPredictionModel::p(Mixer& m,int val1,int val2){
  if (x.bpos==0) {
    const U8 W=buf(1), WW=buf(2), WWW=buf(3);
    int i=0;
    for (; i<nOLS; i++)
      ols[i].Update(W);
    for (i=1; i<=32; i++) {
      ols[0].Add(buf(i));
      ols[1].Add(buf(i*2-1));
      ols[2].Add(buf(i*2));
    }
    for (i=0; i<nOLS; i++)
      prd[i]=Clip(floor(ols[i].Predict()));
    prd[i++]=Clip(W*2-WW);
    prd[i  ]=Clip(W*3-WW*3+WWW);
  }
  const U8 B=x.c0<<(8-x.bpos);
  for (int i=0; i<nLnrPrd; i++) {
    sMap[i].set((prd[i]-B)*8+x.bpos);
    sMap[i].mix(m, 6, 1, 2);
  }
  return 0;
}
