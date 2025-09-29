#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
//#include "../prt/stationarymap.hpp"
//#include "../prt/indirect.hpp"
#include "../prt/indirectcontext.hpp"
//#include "../prt/contextmap2.hpp"
#include "../prt/APM.hpp"
#include "../prt/LSTM.hpp"
// LSTM
//#include "lstm1.inc"
class lstmModel1: public Model {
  BlockData& x;
  Buf& buf;
  APM apm1,apm2,apm3;
  const int horizon;
  LSTM::ByteModel *lstm;
  IndirectContext1<std::uint16_t> iCtx;
public:
  lstmModel1(BlockData& bd,U32 val=0);
 int inputs() {return 2+1+1+1;}
 int nets() {return (horizon<<3)+7+1+8*256;}
 int netcount() {return 1+1;}
 int p(Mixer& m,int val1=0,int val2=0);
  virtual ~lstmModel1(){ delete lstm;}
};
