#include "predictorimg8.hpp"
// 8-bit image predicor
extern bool slow;

PredictorIMG8::PredictorIMG8(): pr(16384),  Image{
     {{0x1000, 0x10000, 0x10000, 0x10000},  {{0x10000,x}, {0x10000,x}}},
     {0x1000, 0x10000, 0x10000} } ,StateMaps{ 256, 256*256}, sse(x){
  loadModels(activeModels,4);  
   // add extra 
   mixerInputs+=1+2;
   sse.p(pr);x.count=0x1ffff;
   // create mixer
   m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount,0,3,2);
}

void PredictorIMG8::update()  {
  pr=(32768-pr)/(32768/4096);
  if(pr<1) pr=1;
  if(pr>4095) pr=4095;
  x.Misses+=x.Misses+((pr>>11)!=x.y);
  m->update();
  m->add(256);
  if (x.bpos==0)x.count++;
  models[M_MATCH]->p(*m);
  models[M_MATCH1]->p(*m);
  //if (slow==true) models[M_NORMAL]->p(*m);
  models[M_IM8]->p(*m,x.finfo);
  if (slow==true) models[M_LSTM]->p(*m);
  m->add((stretch(StateMaps[0].p(x.c0,x.y))+1)>>1);
  m->add((stretch(StateMaps[1].p(x.c0|(x.buf(1)<<8),x.y))+1)>>1);
  
   if(x.filetype== IMAGE8GRAY)  {
      int pr0=m->p(0,1);
      int pr1, pr2, pr3;
      int limit=0x3FF>>((x.blpos<0xFFF)*4);
      pr  = Image.Gray.APMs[0].p(pr0, (x.c0<<4)|(x.Misses&0xF), x.y,limit);
      pr1 = Image.Gray.APMs[1].p(pr, (x.c0<<8)|x.Image.ctx, x.y,limit);
      pr2 = Image.Gray.APMs[2].p(pr0, x.bpos|(x.Image.ctx&0xF8)|(x.Match.byte<<8),x.y, limit);
      pr0 = (2*pr0+pr1+pr2+2)>>2;
      pr = (pr+pr0+1)>>1; 
      }
  else {
       int pr0=m->p(1,1);
      int pr1, pr2, pr3;
      int limit=0x3FF>>((x.blpos<0xFFF)*4);
      pr  = Image.Palette.APMs[0].p(pr0, (x.c0<<4)|(x.Misses&0xF), x.y, limit);
      pr1 = Image.Palette.APMs[1].p(pr0, hash(x.c0, x.Image.pixels.W, x.Image.pixels.N)&0xFFFF,x.y, limit);
      pr2 = Image.Palette.APMs[2].p(pr0, hash(x.c0, x.Image.pixels.N, x.Image.pixels.NN)&0xFFFF,x.y, limit);
      pr3 = Image.Palette.APMs[3].p(pr0, hash(x.c0, x.Image.pixels.W, x.Image.pixels.WW)&0xFFFF,x.y, limit);
      pr0 = (pr0+pr1+pr2+pr3+2)>>2;
      
      pr1 = Image.Palette.APM1s[0].p(pr0, hash(x.c0, x.Match.byte, x.Image.pixels.N)&0xFFFF, 5);
      pr2 = Image.Palette.APM1s[1].p(pr, hash(x.c0, x.Image.pixels.W, x.Image.pixels.N)&0xFFFF, 6);
      pr = (pr*2+pr1+pr2+2)>>2;
      pr = (pr+pr0+1)>>1;   
  }
  sse.update();
  pr = sse.p(pr);
}

