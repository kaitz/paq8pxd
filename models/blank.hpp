#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
//#include "../prt/array.hpp"
#include "../prt/mixer.hpp"

//
class blankModel1: public Model {
  //BlockData& x;
  //Buf& buf;
public:
  blankModel1(BlockData& bd,U32 val=0)/*:x(bd),buf(bd.buf)*/{ }
  int inputs() {return 0;}
  int nets() {return 0;}
  int netcount() {return 0;}
  inline int p(Mixer& m,int val1=0,int val2=0){  
    return 0;
  }
  virtual ~blankModel1(){}
};
