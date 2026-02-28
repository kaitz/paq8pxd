#include "ppmd.hpp"
//#include "mod_ppmd.inc"
ppmdModel1::ppmdModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf){
    int ppmdmem=((210<<(x.settings.level>8))<<(x.settings.level>9))<<(x.settings.level>10);
    ppmd_12_256_1.Init(12+(x.settings.level>8?4:0),ppmdmem,1,0);
    ppmd_6_64_2.Init(6,64,1,0);
}
 
int ppmdModel1::p(Mixers& m,int val1,int val2){
  m.add(stretch(4096-ppmd_12_256_1.ppmd_Predict(4096,x.y)));
  m.add(stretch(4096-ppmd_6_64_2.ppmd_Predict(4096,x.y)));
  return 0;
}
  
