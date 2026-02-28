#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// DECAlpha predicor
class PredictorDEC: public Predictors {
  int pr;
  int pr0;
  int order;
  int ismatch;
  Mixers *m; 
  struct {
      APM APMs[1];
    } DEC;
  eSSE sse;
  const std::vector<ModelTypes> activeModels { 
   M_RECORD,
   M_MATCH ,
   M_MATCH1, 
   M_DISTANCE,  
   M_INDIRECT, 
   M_DMC, 
   M_NEST, 
   M_NORMAL,  
   M_TEXT, 
   M_WORD ,
   M_DEC,  
   M_SPARSEMATCH, 
   M_SPARSE_Y,
   M_CHART,M_LSTM    };
public:
     int mcxt[8];
  PredictorDEC(Settings &set);
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
  void update() ;
   ~PredictorDEC(){
  }
};

