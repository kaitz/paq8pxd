#pragma once
#include "predictors.hpp"
#include "../prt/mixer.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// x86/64 predicor
class PredictorEXE: public Predictors {
  int pr;  // next prediction
  int order;
  Mixer *m;
  struct {
      APM APMs[3];
    } x86_64;
  U32 count;
  eSSE sse;
  const U8 activeModels[17] = { 
   M_RECORD,
   M_MATCH ,
   M_MATCH1, 
   M_DISTANCE, 
   M_EXE, 
   M_INDIRECT, 
   M_DMC, 
   M_NEST, 
   M_NORMAL, 
   M_XML, 
   M_TEXT, 
   M_WORD ,
   M_SPARSEMATCH, 
   M_SPARSE_Y,
   M_PPM,M_CHART,M_LSTM };
public:
  PredictorEXE();
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
  void update() ;
    ~PredictorEXE(){
    }
};


