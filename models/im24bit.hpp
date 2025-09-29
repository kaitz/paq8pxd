#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/stationarymap.hpp"
//#include "../prt/indirect.hpp"
//#include "../prt/indirectcontext.hpp"
#include "../prt/contextmap2.hpp"
#include "../prt/sscm.hpp"
#include "../prt/ols.hpp"

//////////////////////////// im24bitModel /////////////////////////////////
// Model for 24-bit image data

#define nSCMaps 59
#define n2Maps1 76
#define n2Maps 100

class im24bitModel1: public Model {
 int nOLS ;
 int inpts;
 ContextMap2 cm;
 int col, color,stride;
 int ctx[2];
 int padding, x;
 int columns[2];
 int column[2];
 BlockData& xx;
 Buf& buf; 
 RingBuffer buffer;// internal rotating buffer for PNG unfiltered pixel data
 U8 px  ; // current PNG filter prediction
 int filter, w, line, isPNG,R1, R2;
 bool filterOn;
 U32& c4;
 int& c0;
 int& bpos;
 int lastWasPNG;
 U8 WWp1, Wp1, p1, NWp1, Np1, NEp1, NNp1 ;
 U8 WWp2, Wp2, p2, NWp2, Np2, NEp2, NNp2;
 U32 lastw,lastpos,curpos;
 U8 WWWWWW, WWWWW, WWWW, WWW, WW, W;
   U8 NWWWW, NWWW, NWW, NW, N, NE, NEE, NEEE, NEEEE;
   U8 NNWWW, NNWW, NNW, NN, NNE, NNEE, NNEEE;
   U8 NNNWW, NNNW, NNN, NNNE, NNNEE;
   U8 NNNNW, NNNN, NNNNE;
   U8 NNNNN;
   U8 NNNNNN;
   Array<U8> MapCtxs, SCMapCtxs, pOLS;
   const double lambda[6] ={ 0.98, 0.87, 0.9, 0.8, 0.9, 0.7 };
   const int num[6] ={ 32, 12, 15, 10, 14, 8 };
   OLS<double, U8> ols[6][4] = { 
    {{num[0], 1, lambda[0]}, {num[0], 1, lambda[0]}, {num[0], 1, lambda[0]}, {num[0], 1, lambda[0]}},
    {{num[1], 1, lambda[1]}, {num[1], 1, lambda[1]}, {num[1], 1, lambda[1]}, {num[1], 1, lambda[1]}},
    {{num[2], 1, lambda[2]}, {num[2], 1, lambda[2]}, {num[2], 1, lambda[2]}, {num[2], 1, lambda[2]}},
    {{num[3], 1, lambda[3]}, {num[3], 1, lambda[3]}, {num[3], 1, lambda[3]}, {num[3], 1, lambda[3]}},
    {{num[4], 1, lambda[4]}, {num[4], 1, lambda[4]}, {num[4], 1, lambda[4]}, {num[4], 1, lambda[4]}},
    {{num[5], 1, lambda[5]}, {num[5], 1, lambda[5]}, {num[5], 1, lambda[5]}, {num[5], 1, lambda[5]}}
  };
   const U8 *ols_ctx1[32] = { &WWWWWW, &WWWWW, &WWWW, &WWW, &WW, &W, &NWWWW, &NWWW, &NWW, &NW, &N, &NE, &NEE, &NEEE, &NEEEE, &NNWWW, &NNWW, &NNW, &NN, &NNE, &NNEE, &NNEEE, &NNNWW, &NNNW, &NNN, &NNNE, &NNNEE, &NNNNW, &NNNN, &NNNNE, &NNNNN, &NNNNNN };
   const U8 *ols_ctx2[12] = { &WWW, &WW, &W, &NWW, &NW, &N, &NE, &NEE, &NNW, &NN, &NNE, &NNN }; 
   const U8 *ols_ctx3[15] = { &N, &NE, &NEE, &NEEE, &NEEEE, &NN, &NNE, &NNEE, &NNEEE, &NNN, &NNNE, &NNNEE, &NNNN, &NNNNE, &NNNNN };
   const U8 *ols_ctx4[10] = { &N, &NE, &NEE, &NEEE, &NN, &NNE, &NNEE, &NNN, &NNNE, &NNNN };
   const U8 *ols_ctx5[14] = { &WWWW, &WWW, &WW, &W, &NWWW, &NWW, &NW, &N, &NNWW, &NNW, &NN, &NNNW, &NNN, &NNNN };
   const U8 *ols_ctx6[ 8] = { &WWW, &WW, &W, &NNN, &NN, &N, &p1, &p2 };
   const U8 **ols_ctxs[6] = { &ols_ctx1[0], &ols_ctx2[0], &ols_ctx3[0], &ols_ctx4[0], &ols_ctx5[0], &ols_ctx6[0] };

    SmallStationaryContextMap SCMap[nSCMaps] = { {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                                      {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                                      {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                                      {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                                      {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                                      {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                                      {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                                      {11,1}, {11,1}, { 0,8}};
    StationaryMap Map[n2Maps] ={      8,      8,      8,      2,      0, {15,1}, {15,1}, {15,1}, {15,1}, {15,1},
                                     {17,1}, {17,1}, {17,1}, {17,1}, {13,1}, {13,1}, {13,1}, {13,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},{11,1}, {11,1}, {11,1}, {11,1}};
    Array<U32> mixCxt;
public:
  im24bitModel1(BlockData& bd);
   
  int inputs() {return inpts*cm.inputs()+nSCMaps*2+100*2+1;}
  int nets() {return 256+   256+   512+   2048+   8*32+   6*64+   256*2+   1024+   8192+   8192+   8192+   8192+  256;}
  int netcount() {return 13;}
   
  int p(Mixer& m,int info,int val2=0);

  // Square buf(i)
inline int sqrbuf(int i) {
  assert(i>0);
  return buf(i)*buf(i);
}
  virtual ~im24bitModel1(){ }
 
};


