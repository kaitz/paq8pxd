#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixers.hpp"
#include "../prt/log.hpp"
#include "model.hpp"
#include "../prt/stationarymap.hpp"
//////////////////////////// distanceModel ///////////////////////
 
// Model for modelling distances between symbols
class distanceModel1: public Model {
  int pos00,pos20,posnl;
  BlockData& x;
  Buf& buf;
  StationaryMap Maps[3];
public:
    int mxcxt[3];
  distanceModel1(BlockData& bd);
  int inputs() {return 3*2;}
  int nets() {return 256*3;}
  int netcount() {return 3;}
  int p(Mixers& m,int val1=0,int val2=0);
virtual ~distanceModel1(){ }
}; 
