#include "predictorimg4.hpp"
// 4-bit image predicor
extern bool slow;

PredictorIMG4::PredictorIMG4(): pr(16384), StateMaps{ 256, 256*256}, Image
     {{0x1000, 0x8000, 0x8000, 0x8000},  {{0x10000,x}, {0x10000,x}}}, sse(x) {
   loadModels(activeModels,4);  
   // add extra 
   mixerInputs+=1+2;
   mixerNets+=0;
   mixerNetsCount+=0;
   sse.p(pr);
   // create mixer
   m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount);
}

void PredictorIMG4::update()  {
  pr=(32768-pr)/(32768/4096);
  if(pr<1) pr=1;
  if(pr>4095) pr=4095;
  x.Misses+=x.Misses+((pr>>11)!=x.y);
  m->update();
  m->add(256);
  models[M_MATCH]->p(*m);
  models[M_MATCH1]->p(*m);
  models[M_IM4]->p(*m,x.finfo);
  if (slow==true) models[M_LSTM]->p(*m);
  m->add((stretch(StateMaps[0].p(x.c0,x.y))+1)>>1);
  m->add((stretch(StateMaps[1].p(x.c0|(x.buf(1)<<8),x.y))+1)>>1);
  int pr0=m->p();
  int pr1, pr2, pr3;
  int limit=0x3FF>>((x.blpos<0xFFF)*4);
  pr  = Image.APMs[0].p(pr0, (x.c0<<4)|(x.Misses&0xF), x.y, limit);
  pr1 = Image.APMs[1].p(pr0, hash(x.c0, x.Image.pixels.W, x.Image.pixels.N)&0x7FFF,x.y, limit);
  pr2 = Image.APMs[2].p(pr0, hash(x.c0, x.Image.pixels.N, x.Image.pixels.NN)&0x7FFF,x.y, limit);
  pr3 = Image.APMs[3].p(pr0, hash(x.c0, x.Image.pixels.W, x.Image.pixels.WW)&0x7FFF,x.y, limit);
  pr0 = (pr0+pr1+pr2+pr3+2)>>2;
      
  pr1 = Image.APM1s[0].p(pr0, hash(x.c0, x.Match.byte, x.Image.pixels.N)&0xFFFF, 5);
  pr2 = Image.APM1s[1].p(pr, hash(x.c0, x.Image.pixels.W, x.Image.pixels.N)&0xFFFF, 6);
  pr = (pr*2+pr1+pr2+2)>>2;
  pr = (pr+pr0+1)>>1;   
  sse.update();
  pr = sse.p(pr);
}

