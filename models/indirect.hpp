#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/indirectcontext.hpp"
//#include "../prt/contextmap.hpp"
#include "../prt/CM128.hpp"
//////////////////////////// indirectModel /////////////////////

// The context is a byte string history that occurs within a
// 1 or 2 byte context.
class indirectModel1: public Model {
  BlockData& x;
  Buf& buf;
  const int N;
  const int mem;
  ContextMap3 cm2;
  ContextMap3 cm3;
  ContextMap3 cm4;
  ContextMap3 cm5;
  ContextMap3 cmt;
  ContextMap3 cm0;
  ContextMap3 cma;
  ContextMap3 cmc;
  Array<U32> t1;
  Array<U16> t2;
  Array<U16> t3;
  Array<U16> t4;
  Array<U32> t5; // 256K
  IndirectContext<U32> iCtx;
  LargeIndirectContext<uint32_t> iCtxLarge{ 18,8 }; // 11MB // hashBits, inputBits
  U32 chars4; 
public:
  indirectModel1(BlockData& bd,U32 val=0);
  int inputs() {return (4*3+13)*cm2.inputs()+2;}
  int nets() {return 0;}
  int netcount() {return 0;}
  void setContexts(){
      
  }
int p(Mixer& m,int val1=0,int val2=0);
virtual ~indirectModel1(){ }
};
