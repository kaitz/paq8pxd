#pragma once
#include "Predictors.hpp"
#include "../prt/mixer.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"


// General predicor class
class Predictor: public Predictors {
  int pr;  // next prediction
  int pr0;
  int order;
  int ismatch;
  Mixer *m;
  EAPM a;
  bool isCompressed; 
  U32 count;
  U32 lastmiss;
  eSSE sse;
  const U8 activeModels[18] = { 
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
   M_LINEAR, 
   M_SPARSEMATCH, 
   M_SPARSE_Y,
   M_PPM,M_CHART,M_LSTM    };
public:
  Predictor();
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
  void update() ;
   ~Predictor(){
        delete m;
  }
};


