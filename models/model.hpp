#pragma once
#include "../prt/types.hpp"
#include "../prt/mixer.hpp"

class Model {
public:
  virtual  int p(Mixer& m,int val1=0,int val2=0)=0;
  virtual  int inputs()=0;
  virtual  int nets()=0;
  virtual  int netcount()=0;
  virtual  void setword(U8 *w,int len=0){} ;
  virtual ~Model(){};
};
