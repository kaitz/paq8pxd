#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/statemap.hpp"
#include "../prt/bh.hpp"
//////////////////////////// im4bitModel /////////////////////////////////

// Model for 4-bit image data
class im4bitModel1: public Model {
    BlockData& x;
    Buf& buf;
    BH<16> t;
    const int S; // number of contexts
    Array<U8*> cp;
    StateMap *sm;
    StateMap map;
    U8 WW, W, NWW, NW, N, NE, NEE, NNWW, NNW, NN, NNE, NNEE;
    int col, line, run, prevColor, px;
    public:
 im4bitModel1( BlockData& bd,U32 val=0 );
  int inputs() {return S*3+2;}
  int nets() {return 256+   512+   512+  1024+   16+  1;}
  int netcount() {return 6;}
   
int p(Mixer& m,int w=0,int val2=0);
 virtual ~im4bitModel1(){  delete[] sm;}
 
};
