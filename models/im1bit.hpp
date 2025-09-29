#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/statemap.hpp"
#include "../prt/bh.hpp"
//#include <cmath>
//////////////////////////// im1bitModel /////////////////////////////////
// Model for 1-bit image data
class im1bitModel1: public Model {
   BlockData& x;
   Buf& buf;
   U32 r0, r1, r2, r3;  // last 4 rows, bit 8 is over current pixel
   Array<U8> t;  // model: cxt -> state
   const int N;  // number of contexts
   Array<int>  cxt;  // contexts
   StateMap* sm;
   BH<4> t1;
  U8* cp;
public:
  im1bitModel1( BlockData& bd,U32 val=0 );
  int inputs() {return N+2;}
  int nets() {return 256+256+256+256+256;}
  int netcount() {return 5;}
int p(Mixer& m,int w=0,int val2=0) ;
 virtual ~im1bitModel1(){  delete[] sm;}
 
};

