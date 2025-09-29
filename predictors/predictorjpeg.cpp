#include "predictorjpeg.hpp"
// Jpeg predicor
extern bool slow;

PredictorJPEG::PredictorJPEG(): pr(16384), 
  Jpeg{
    { /*APM:*/ { 0x2000} }
  },
  Bypass(false){
   loadModels(activeModels,5);
   // add extra 
   mixerInputs+=3+1+1-3;
   mixerNets+=        (8+1024)+       256+       256+       256+       256+       1536;
     
   mixerNetsCount+=6;
   // create mixer
   m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount,0,3,2); //set  error update rate to 1.5 (3/2), default is 7/1
}

void PredictorJPEG::update()  {
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    x.Misses+=x.Misses+((pr>>10)!=x.y);
    m->update();
    m->add(256);
    Bypass=false;
    int ismatch=models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    if (slow==true) models[M_LSTM]->p(*m);
    if (slow==false && (x.Match.length>0xFF || x.Match.bypass)) {//256b
        x.Match.bypass =   Bypass =    true;
        pr= x.Match.bypassprediction;
        pr=(4096-pr)*(32768/4096);
        if(pr<1) pr=1;
        if(pr>32767) pr=32767;
        models[M_JPEG]->p(*m,1);//we found long repeating match. update, do not predict. artificial images, same partial content
        m->reset(); 
        return;
    }
    if (models[M_JPEG]->p(*m)) {
        if (slow==true) models[M_NORMAL]->p(*m);
        pr=m->p(1,0);
    }
    else{
        int order =models[M_NORMAL]->p(*m);
        U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
        m->set(8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512, 8+1024);
        m->set(x.c0, 256);
        m->set(order | ((x.c4>>6)&3)<<3 | (x.bpos==0)<<5 | (c1==c2)<<6 | (1)<<7, 256);
        m->set(c2, 256);
        m->set(c3, 256);
        U8 d=x.c0<<(8-x.bpos);
        if (x.bpos) {
            c=d; if (x.bpos==1)c+=c3/2;
            c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
        }
        else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
        m->set(c, 1536);
        pr=m->p(1,1);
    }
    U32 pr0 = Jpeg.APMs[0].p(pr , x.JPEG.state,x.y, 0x3FF);
    pr = (pr + pr0 + 1) / 2;
    pr=(4096-pr)*(32768/4096);
    if(pr<1) pr=1;
    if(pr>32767) pr=32767;
}

