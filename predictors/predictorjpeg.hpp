#pragma once
#include "Predictors.hpp"
#include "../prt/mixer.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// Jpeg predicor
class PredictorJPEG: public Predictors {
  int pr;
  Mixer *m;
  struct {
      APM APMs[1];
    } Jpeg;
  bool Bypass; 
  const U8 activeModels[5] = { 
   M_JPEG,
   M_MATCH ,
   M_MATCH1, 
   M_NORMAL, 
   M_LSTM    };
public:
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
  ~PredictorJPEG(){   //printf("\n JPEG Count of skipped bytes %d\n",x.count);
      
 }
PredictorJPEG();

void update() ;
};
