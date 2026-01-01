#pragma once
#include "predictors.hpp"
#include "../prt/mixer.hpp"
//#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// 1-bit image predicor
class PredictorIMG1: public Predictors {
  int pr;  // next prediction
  Mixer *m;
  eSSE sse;
  const U8 activeModels[4] = { 
   M_MATCH ,
   M_MATCH1, 
   M_IM1,
   M_LSTM    };
public:
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
   ~PredictorIMG1(){ }

PredictorIMG1();

void update();
};
