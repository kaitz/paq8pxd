#pragma once
#include "Predictors.hpp"
#include "../prt/mixer.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// 24/32-bit image predicor
class PredictorIMG24: public Predictors {
  int pr;  // next prediction
  Mixer *m;
  struct {
    APM APMs[1];
    APM1 APM1s[2];
  } Image;
  StateMap StateMaps[2];
  eSSE sse;
  const U8 activeModels[4] = { 
   M_MATCH ,
   M_MATCH1, 
   M_IM24,
   //M_NORMAL,
   M_LSTM    };
public:
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
   ~PredictorIMG24(){ }

PredictorIMG24();

void update();
};
