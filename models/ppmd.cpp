#include "ppmd.hpp"
//#include "mod_ppmd.inc"
extern U8 level;
  ppmdModel1::ppmdModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf){
    int ppmdmem=((210<<(level>8))<<(level>9))<<(level>10);
    ppmd_12_256_1.Init(12+(level>8?4:0),ppmdmem,1,0);
    ppmd_6_64_2.Init(6,64,1,0);
 }
 
int ppmdModel1::p(Mixer& m,int val1,int val2){
  m.add(stretch(4096-ppmd_12_256_1.ppmd_Predict(4096,x.y)));
  m.add(stretch(4096-ppmd_6_64_2.ppmd_Predict(4096,x.y)));
  return 0;
}
  
