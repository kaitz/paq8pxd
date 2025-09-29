#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/sscm.hpp"
#include "../prt/ols.hpp"
//#include <cmath>

class linearPredictionModel: public Model {
  BlockData& x;
  Buf& buf;
   int nOLS, nLnrPrd;
SmallStationaryContextMap sMap[3+2]{ {11,1},{11,1},{11,1},{11,1},{11,1} };
  OLS<double, U8> ols[3]{ {32, 4, 0.995}, {32, 4, 0.995}, {32, 4, 0.995} };
  U8 prd[5]{ 0 };
public:
  linearPredictionModel(BlockData& bd,U32 val=0);
  int inputs() {return nLnrPrd*2;}
  int nets() {return 0;}
  int netcount() {return 0;}
  int p(Mixer& m,int val1=0,int val2=0);
virtual ~linearPredictionModel(){ }
};
