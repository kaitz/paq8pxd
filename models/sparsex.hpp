#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/sscm.hpp"
#include "../prt/contextmap.hpp"

class sparseModelx: public Model {
   const int N;
   ContextMap cm;
   SmallStationaryContextMap scm1, scm2, scm3,
   scm4, scm5,scm6, scma;
   BlockData& x;
   Buf& buf;
public:
  sparseModelx(BlockData& bd);
    int inputs() {return N*cm.inputs()+7*2;}
    int nets() {return 0;}
  int netcount() {return 0;}
  int p(Mixer& m, int seenbefore, int howmany);
 virtual ~sparseModelx(){ }
};
