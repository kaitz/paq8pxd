#pragma once
#include "Predictors.hpp"
#include "../prt/mixer.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"

// Text predicor
class PredictorTXTWRT: public Predictors {
  int pr;  // next prediction
  int pr0;
  int order;
  int rlen;
  int ismatch;
  Mixer *m;
  struct {
    APM APMs[4];
    APM1 APM1s[3];
  } Text;
  U32 count;
  U32 blenght;
  StateMap StateMaps[1];
  wrtDecoder wr;
  eSSE sse;
  int decodedTextLen,lasttag;
  int counttags,lState;
  const U8 activeModels[15] = { 
   M_RECORD,
   M_MATCH ,
   M_MATCH1,  
   M_INDIRECT, 
   M_DMC, 
   M_NEST, 
   M_NORMAL, 
   M_XML, 
   M_TEXT, 
   M_WORD , 
   M_SPARSEMATCH, 
   M_SPARSE,
   M_PPM,M_CHART,M_LSTM    };
public:
  PredictorTXTWRT();
  int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
  void wrt();
  void update() ;
   ~PredictorTXTWRT(){
      // printf("\n TXTWRT Count of skipped bytes %d\n",count/8);
  }
};


