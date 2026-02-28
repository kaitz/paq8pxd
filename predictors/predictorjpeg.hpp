#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// Jpeg predicor
class PredictorJPEG: public Predictors {
  int pr;
  Mixers *m;
  struct {
      APM APMs[1];
    } Jpeg;
  bool Bypass; 
  const std::vector<ModelTypes> activeModels { 
   M_JPEG,
   M_MATCH ,
   M_MATCH1, 
   M_NORMAL, 
   M_LSTM    };
public:
    int mcxt[9];
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
  ~PredictorJPEG(){   //printf("\n JPEG Count of skipped bytes %d\n",x.count);
      
 }
PredictorJPEG(Settings &set);

void update() ;
};
