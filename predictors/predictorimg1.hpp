#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
//#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// 1-bit image predicor
class PredictorIMG1: public Predictors {
  int pr;  // next prediction
  Mixers *m;
  eSSE sse;
  const std::vector<ModelTypes> activeModels { 
   M_MATCH ,
   M_MATCH1, 
   M_IM1,
   M_LSTM    };
public:
     int mcxt[2];
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
   ~PredictorIMG1(){ }

PredictorIMG1(Settings &set);

void update();
};
