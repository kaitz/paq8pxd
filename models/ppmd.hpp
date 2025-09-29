#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
//#include "../prt/hash.hpp"
#include "model.hpp"
//#include "../prt/stationarymap.hpp"
//#include "../prt/indirect.hpp"
//#include "../prt/indirectcontext.hpp"
//#include "../prt/contextmap2.hpp"
//#include "../prt/sscm.hpp"
//#include "../prt/ols.hpp"
//#include "../prt/wrt/wrton.hpp"
//#include "../prt/stemmer/stemmer.hpp"
#include "../prt/mod_ppmd.hpp"
//#include "mod_ppmd.inc"

class ppmdModel1: public Model {
  BlockData& x;
  Buf& buf;
  ppmd_Model ppmd_12_256_1;
  ppmd_Model ppmd_6_64_2;
public:
  ppmdModel1(BlockData& bd,U32 val=0);
 int inputs() {return 2;}
 int nets() {return 0;}
  int netcount() {return 0;}
int p(Mixer& m,int val1=0,int val2=0);
  virtual ~ppmdModel1(){ }
};

