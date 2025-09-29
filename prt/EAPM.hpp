#pragma once
#include "types.hpp"
//#include "../prt/helper.hpp"
//#include "array.hpp"
//#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
//#include "model.hpp"
//#include "../prt/stationarymap.hpp"
//#include "../prt/indirect.hpp"
//#include "../prt/indirectcontext.hpp"
#include "../prt/APM1.hpp"
//#include "../prt/sscm.hpp"
//#include "../prt/ols.hpp"
//#include "../prt/wrt/wrton.hpp"
//#include "../prt/stemmer/stemmer.hpp"
#include "../prt/tables.hpp"
// Filter the context model with APMs
class EAPM {
  BlockData& x;
  APM1 a, a1, a2, a3, a4, a5, a6;
public:
  EAPM(BlockData& bd);
  int p1(int pr0,int pr, int r) ;
  int p2(int pr0,int pr, int r) ;
   ~EAPM(){
  }
};


