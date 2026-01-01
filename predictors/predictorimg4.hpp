#pragma once
#include "predictors.hpp"
#include "../prt/mixer.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"

// 4-bit image predicor
class PredictorIMG4: public Predictors {
  int pr;
  Mixer *m;
  StateMap StateMaps[2];
  struct {
      APM APMs[4];
      APM1 APM1s[2];
  } Image;
  eSSE sse;
  const U8 activeModels[4] = { 
   M_MATCH ,
   M_MATCH1, 
   M_IM4,
   M_LSTM    };
public:
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
   ~PredictorIMG4(){ }

PredictorIMG4();

void update();
};
