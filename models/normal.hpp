static const int primes[17]={ 0, 257,251,241,239,233,229,227,223,211,199,197,193,191,181,179,173};   
#pragma once
#include "../prt/types.hpp"
#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
//#include "../prt/mixer.hpp"
//#include "../prt/hash.hpp"
#include "model.hpp"
//#include "../prt/indirectcontext.hpp"
#include "../prt/statemap.hpp"
//#include "../prt/stationarymap.hpp"
//#include "../prt/sscm.hpp"
#include "../prt/contextmap2.hpp"
#include "../prt/run.hpp"


class normalModel1: public Model {
  BlockData& x;
  Buf& buf;
  const int N;
  ContextMap2   cm;
  StateMap StateMaps[4];
  RunContextMap /*rcm7,*/ rcm9, rcm10;
  int inpt;
public:
  normalModel1(BlockData& bd,U32 val=0);
 int inputs() {return 10*cm.inputs() +3+2+2+1;}
 int nets() {return 0;}
  int netcount() {return 0;}
  
  
int p(Mixer& m,int val1=0,int val2=0);
  virtual ~normalModel1(){

 }
};
