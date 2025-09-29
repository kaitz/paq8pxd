#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/contextmap.hpp"

class nestModel1: public Model {
  BlockData& x;
  Buf& buf;
  int ic, bc, pc,vc, qc, lvc, wc,ac, ec, uc, sense1, sense2, w;
  const int N;
  ContextMap cm;
public:
  nestModel1(BlockData& bd,U32 val=0);
  int inputs() {return N*cm.inputs();}
  int nets() {return 512;}
  int netcount() {return 1;}
int p(Mixer& m,int val1=0,int val2=0);
virtual ~nestModel1(){ }
};


