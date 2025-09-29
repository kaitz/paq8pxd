#pragma once
#include "blockdata.hpp"
//#include "../prt/helper.hpp"
//#include "array.hpp"
//#include "../prt/mixer.hpp"
//#include "../prt/hash.hpp"
//#include "model.hpp"
//#include "../prt/stationarymap.hpp"
//#include "../prt/indirect.hpp"
//#include "../prt/indirectcontext.hpp"
#include "mod_sse.hpp"
//#include "../prt/sscm.hpp"
//#include "../prt/ols.hpp"
//#include "../prt/wrt/wrton.hpp"
//#include "../prt/stemmer/stemmer.hpp"
//#include "../prt/tables.hpp"
// Extra SSE
//#include "mod_sse.cpp"
using SSE_sh::M_T1;

class eSSE {
  BlockData& x;
 SSE_sh::M_T1* sse_;
public:
  eSSE(BlockData& bd);
  int p(int input) ;
  void update() ;
   ~eSSE(){ delete sse_;
  }
};

