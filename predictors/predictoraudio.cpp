#include "predictoraudio.hpp"
// Audio predicor

PredictorAUDIO2::PredictorAUDIO2(): pr(16384),a(x), sse(x) {
   loadModels(activeModels,3);  
   // add extra 
   mixerInputs+=1;
   mixerNets+=0;
   mixerNetsCount+=0;
   sse.p(pr);
   // create mixer
   m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount);
}

void PredictorAUDIO2::update()  {
  m->update();
  m->add(256);
  models[M_MATCH]->p(*m);  
  models[M_RECORD]->p(*m);
  models[M_WAV]->p(*m,x.finfo);
  pr=(32768-pr)/(32768/4096);
  if(pr<1) pr=1;
  if(pr>4095) pr=4095;
  pr=a.p1(m->p(((x.finfo&2)==0),1),pr,7);
  sse.update();
    pr = sse.p(pr);
}

