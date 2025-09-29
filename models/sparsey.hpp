#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/contextmap2.hpp"
//////////////////////////// sparseModel ///////////////////////

// Model order 1-2 contexts with gaps.
class sparseModely: public Model {
  BlockData& x;
  Buf& buf;
  const int N;
  ContextMap2 cm;
  U32 ctx;
public:
  sparseModely(BlockData& bd,U32 val=0);
  int inputs() {return N*cm.inputs();}
  int nets() {return 4 * 256;}
  int netcount() {return 1;}
  int p(Mixer& m,int seenbefore,int howmany);
virtual ~sparseModely(){ }
};
