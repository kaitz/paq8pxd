#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/indirect.hpp"
#include "../prt/indirectcontext.hpp"
#include "../prt/contextmap.hpp"
#include "../prt/sscm.hpp"
#include "../prt/stationarymap.hpp"
#include "../prt/helper.hpp"
//////////////////////////// recordModel ///////////////////////

// Model 2-D data with fixed record length.  Also order 1-2 models
// that include the distance to the last match.


class recordModel1: public Model {
   BlockData& x;
   Buf& buf;
   Array<int> cpos1, cpos2, cpos3, cpos4;
   Array<int> wpos1; // buf(1..2) -> last position
   Array<int> rlen;//, rlen1, rlen2, rlen3,rlenl;  // run length and 2 candidates
   Array<int>  rcount;//, rcount2,rcount3;  // candidate counts
    U8 padding; // detected padding byte
   int prevTransition,nTransition  ; // position of the last padding transition
   int col;
   int mxCtx,x1;
   bool MayBeImg24b;
   ContextMap cm, cn, cq, co, cp;
    int nMaps ;
   StationaryMap Maps[6] ={ 10,10,8,8,8,{11,1} };
    SmallStationaryContextMap sMap[3]{ {11, 1}, {3, 1}, {16,1} };
    IndirectMap iMap[3]{ 8,8,8 };
    IndirectContext<U16> iCtx[5]{ {16,8}, {16,8}, {16,8}, {20,8}, {11,1} };
   U8 N, NN, NNN, NNNN,WxNW;
   const int nIndCtxs  ;
public:
  recordModel1( BlockData& bd,U64 msize=CMlimit(MEM()*2) );
  int inputs() {return (3+3+3+3+16)*cm.inputs()+nMaps*2+2+3*2+3*2;}
  int nets() {return 512+11*32+1024;}
  int netcount() {return 2+1;}
  int p(Mixer& m,int rrlen=0,int val2=0);
 virtual ~recordModel1(){ }
};
