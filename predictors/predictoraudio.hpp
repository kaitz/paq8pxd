#pragma once
#include "Predictors.hpp"
#include "../prt/mixer.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// Audio predicor
class PredictorAUDIO2: public Predictors {
  int pr;
  Mixer *m;
  EAPM a;
  eSSE sse;
  const U8 activeModels[3] = {
   M_RECORD ,
   M_MATCH ,    
   M_WAV  };
  void setmixer();
public:
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
   ~PredictorAUDIO2(){  }

PredictorAUDIO2();

void update() ;
};
