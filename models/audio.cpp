#include "audio.hpp"
//////////////////////////// wavModel /////////////////////////////////

///////////////// Least Mean Squares predictor /////////////////



  wavModel1::wavModel1(BlockData& bd): x(bd),buf(bd.buf){
    a8mode=new audio8(bd);
    a16mode=new audio16(bd);
}

int wavModel1::p(Mixer& m,int info,int val2){
    if ((info&2)==0) {
    a8mode->audio8bModel(m, info);
    } 
    else  {
    a16mode->audio16bModel(m, info);
    }
  return 0;
  } 

  
 
