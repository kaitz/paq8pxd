#include "predictorimg1.hpp"
// 1-bit image predicor
extern bool slow;

PredictorIMG1::PredictorIMG1(): pr(16384), sse(x) {
   loadModels(activeModels,4);  
   // add extra 
   mixerInputs+=1;
   mixerNets+=  256;
   mixerNetsCount+=1;
   sse.p(pr);
   // create mixer
   m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount,0,3,2);
}

void PredictorIMG1::update()  {
  m->update();
  m->add(256); 
  int ismatch=models[M_MATCH]->p(*m);
  models[M_MATCH1]->p(*m);
  if (slow==true) models[M_LSTM]->p(*m);
  m->set(ismatch,256);
  models[M_IM1]->p(*m, x.finfo);
  pr=m->p(); 
  sse.update();
    pr = sse.p(pr);
}

