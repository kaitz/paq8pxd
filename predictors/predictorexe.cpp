#include "predictorexe.hpp"

// x86/64 predicor
extern bool slow;

PredictorEXE::PredictorEXE(): pr(16384),order(0),
  x86_64{
    { /*APM:*/ {0x800,20}, {0x10000,16}, {0x10000,16} }
  },
  count(0), sse(x) {
  loadModels(activeModels,17);
   // add extra 
   mixerInputs+=1;
   mixerNets+=( 8+1024)+     256+     256+     256+     256+     256+     1536;
   mixerNetsCount+=7;
   sse.p(pr);
   // create mixer
   m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount);
}

void PredictorEXE::update()  {
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    x.Misses+=x.Misses+((pr>>11)!=x.y);
    if (x.bpos==0) {
        int b1=x.buf(1);
        int i=WRT_mpw[b1>>4];
        x.w4=x.w4*4+i;
        if (x.b2==3) i=2;
        x.w5=x.w5*4+i;
        x.b3=x.b2;
        x.b2=b1;   
        x.x4=x.x4*256+b1,x.x5=(x.x5<<8)+b1;
        if(b1=='.' || b1=='!' || b1=='?' || b1=='/'|| b1==')'|| b1=='}') {
            x.w5=(x.w5<<8)|0x3ff,x.f4=(x.f4&0xfffffff0)+2,x.x5=(x.x5<<8)+b1,x.x4=x.x4*256+b1;
            if(b1!='!') x.w4|=12, x.tt=(x.tt&0xfffffff8)+1,x.b3='.';
        }
        if (b1==32) --b1;
        x.tt=x.tt*8+WRT_mtt[b1];
        x.f4=x.f4*16+(b1>>4);
    }
    m->update();
    m->add(256);
     
    int ismatch=models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    if (x.bpos==0)x.count++;
    models[M_SPARSEMATCH]->p(*m);
    order=models[M_NORMAL]->p(*m);
    int rec=0;
    rec=models[M_RECORD]->p(*m);
    models[M_WORD]->p(*m,order);
    models[M_NEST]->p(*m);
    models[M_SPARSE_Y]->p(*m,ismatch,order);
    models[M_DISTANCE]->p(*m);
    models[M_INDIRECT]->p(*m);
    models[M_DMC]->p(*m);
    models[M_EXE] ->p(*m,1);
    if (slow==false) models[M_XML]->p(*m);
    models[M_TEXT]->p(*m);
    if (slow==true) models[M_PPM]->p(*m); 
    if (slow==true) models[M_CHART]->p(*m);
    if (slow==true) models[M_LSTM]->p(*m);
    U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
    m->set(8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512, 8+1024);
    m->set(x.c0, 256);
    m->set(c2, 256);
    U8 d=x.c0<<(8-x.bpos);
    m->set(order+8*(x.c4>>6&3)+32*(x.bpos==0)+64*(c1==c2)+1*128, 256);
    m->set(c3, 256);
    m->set(ismatch, 256);
    if (x.bpos) {
        c=d; if (x.bpos==1)c+=c3/2;
        c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
    }
    else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
    m->set(c, 1536);
    int pr0=m->p(1,1);
    int const limit = 0x3FFu >> (static_cast<int>(x.blpos < 0xFFFu) * 4);
    pr = x86_64.APMs[0].p(pr0, (x.x86_64.state << 3u) | x.bpos,x.y, limit);
    int  pr1 = x86_64.APMs[1].p(pr0, (x.c0 << 8u) | x.x86_64.state,x.y, limit);
    int  pr2 = x86_64.APMs[2].p((pr1+pr+1)/2, finalize64(hash(x.c4 & 0xFFu, x.bpos, x.Misses & 0x1u, x.x86_64.state >> 3), 16),x.y, limit);
    pr = (pr + pr0 + pr1 + pr2 + 2) >> 2;
    sse.update();
    pr = sse.p(pr);
}
