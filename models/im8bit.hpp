#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/stationarymap.hpp"
#include "../prt/indirect.hpp"
#include "../prt/indirectcontext.hpp"
#include "../prt/contextmap2.hpp"
#include "../prt/sscm.hpp"
#include "../prt/ols.hpp"
//////////////////////////// im8bitModel /////////////////////////////////
// Model for 8-bit image data

class im8bitModel1: public Model {
  typedef enum {
    nOLS = 5,
    nMaps0 = 2,
    nMaps1 = 55,
    nMaps = 62,  //nMaps0 + nMaps1 + nOLS
    nPltMaps = 4
 } im8M;
 int inpts;
 ContextMap2 cm;
 int col;
 BlockData& xx;
 Buf& buf;
 int ctx, lastPos, lastWasPNG, line, x, filter, gray,isPNG,jump;
 int framePos, prevFramePos, frameWidth, prevFrameWidth;
 U32& c4;
 int& c0;
 int& bpos;
 StationaryMap Map[nMaps] = {     0, {15,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1},
                                     {11,1}, {11,1}, {11,1}, {11,1}, {11,1}, {11,1} };
 SmallStationaryContextMap pltMap[nPltMaps] = { {11,1},{11,1},{11,1},{11,1} };
 IndirectMap sceneMap[5]{ {8}, {8}, {22,1}, {11,1}, {11,1} };
 IndirectContext<U8> iCtx[nPltMaps] = { {16,8}, {16,8}, {16,8}, {16,8} };
 U8 px , res,prvFrmPx , prvFrmPred ;// current PNG filter prediction, expected residual
 RingBuffer buffer;// internal rotating buffer for PNG unfiltered pixel data
 bool filterOn;
 int columns[2] ={ 1,1 }  , column[2]={ 1,1 } ;
 Array<short> jumps;
 U8 WWWWWW, WWWWW, WWWW, WWW, WW, W;
    U8 NWWWW, NWWW, NWW, NW, N, NE, NEE, NEEE, NEEEE;
    U8 NNWWW, NNWW, NNW, NN, NNE, NNEE, NNEEE;
    U8 NNNWW, NNNW, NNN, NNNE, NNNEE;
    U8 NNNNW, NNNN, NNNNE;
    U8 NNNNN;
    U8 NNNNNN;
    Array<U8>    MapCtxs ;
    Array<U8>    pOLS;
    const double lambda[nOLS] ={ 0.996, 0.87, 0.93, 0.8, 0.9 };
    const int num[nOLS] ={ 32, 12, 15, 10, 14 };
    OLS<double, U8> ols[nOLS] = { 
    {num[0], 1, lambda[0]},
    {num[1], 1, lambda[1]},
    {num[2], 1, lambda[2]},
    {num[3], 1, lambda[3]},
    {num[4], 1, lambda[4]}
  };
  OLS<double, U8> sceneOls;
    const U8 *ols_ctx1[32] = { &WWWWWW, &WWWWW, &WWWW, &WWW, &WW, &W, &NWWWW, &NWWW, &NWW, &NW, &N, &NE, &NEE, &NEEE, &NEEEE, &NNWWW, &NNWW, &NNW, &NN, &NNE, &NNEE, &NNEEE, &NNNWW, &NNNW, &NNN, &NNNE, &NNNEE, &NNNNW, &NNNN, &NNNNE, &NNNNN, &NNNNNN };
    const U8 *ols_ctx2[12] = { &WWW, &WW, &W, &NWW, &NW, &N, &NE, &NEE, &NNW, &NN, &NNE, &NNN }; 
    const U8 *ols_ctx3[15] = { &N, &NE, &NEE, &NEEE, &NEEEE, &NN, &NNE, &NNEE, &NNEEE, &NNN, &NNNE, &NNNEE, &NNNN, &NNNNE, &NNNNN };
    const U8 *ols_ctx4[10] = { &N, &NE, &NEE, &NEEE, &NN, &NNE, &NNEE, &NNN, &NNNE, &NNNN };
    const U8 *ols_ctx5[14] = { &WWWW, &WWW, &WW, &W, &NWWW, &NWW, &NW, &N, &NNWW, &NNW, &NN, &NNNW, &NNN, &NNNN };
    const U8 **ols_ctxs[nOLS] = { &ols_ctx1[0], &ols_ctx2[0], &ols_ctx3[0], &ols_ctx4[0], &ols_ctx5[0] };
  
public:
  im8bitModel1( BlockData& bd);
  int inputs() {return inpts*cm.inputs()+nMaps*2+nPltMaps*2+5*2;}
  int nets() {return ( 2048+5)+    6*16+    6*32+    256+    1024+    64+    128+    256;}
  int netcount() {return 8;}
  
int p(Mixer& m,int w,int val2=0);
  // Square buf(i)
inline int sqrbuf(int i) {
  assert(i>0);
  return buf(i)*buf(i);
}
  virtual ~im8bitModel1(){ }
 
};


