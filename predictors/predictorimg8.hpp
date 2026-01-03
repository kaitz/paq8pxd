#pragma once
#include "predictors.hpp"
#include "../prt/mixer.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// 8-bit image predicor
class PredictorIMG8: public Predictors {
  int pr;
  Mixer *m;
  struct {
    struct {
      APM APMs[4];
      APM1 APM1s[2];
    } Palette;
      struct {
      APM APMs[3];
    } Gray;
  } Image;
  StateMap StateMaps[2];
  eSSE sse;
  const U8 activeModels[4] = { 
   M_MATCH ,
   M_MATCH1, 
   M_IM8,
   //M_NORMAL,
   M_LSTM    };
public:
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
   ~PredictorIMG8(){ }

PredictorIMG8();

void update() ;
};
