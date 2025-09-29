#pragma once
#include "../prt/types.hpp"
#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/contextmap.hpp"

///////////////////////////// chartModel ////////////////////////////
class chartModel: public Model {
  BlockData& x;
  Buf& buf;
  ContextMap cm,cn;
  Array<U32> chart;
  Array<U8> indirect;
  Array<U8> indirect2;
  Array<U8> indirect3;
public:
  chartModel(BlockData& bd,U32 val=0);
 int inputs() {return 30*cm.inputs()+20*cn.inputs();}
int nets() {return 0;}
  int netcount() {return 0;}
int p(Mixer& m,int val1=0,int val2=0);
  virtual ~chartModel(){ }
};
