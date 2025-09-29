#include "predictordec.hpp"
// DECAlpha predicor
extern bool slow;

PredictorDEC::PredictorDEC(): pr(16384),pr0(pr),order(0),ismatch(0),
  DEC{
    { /*APM:*/ { 25*26,20} }
  },
  sse(x){
   loadModels(activeModels,15);
   // add extra 
   mixerInputs+=1;
   mixerNets+= (8+1024)+       256+   256+   256+    256+    1536+64;
   mixerNetsCount+=7;
   sse.p(pr);
   // create mixer
   m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount,0,3,2);
}

void PredictorDEC::update()  {
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
    ismatch=models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    if (x.bpos==0)x.count++;
    models[M_SPARSEMATCH]->p(*m);
    order=models[M_NORMAL]->p(*m);
    models[M_RECORD]->p(*m,4 );
    models[M_TEXT]->p(*m );
    models[M_WORD]->p(*m );
    models[M_NEST]->p(*m );
    models[M_SPARSE_Y]->p(*m,ismatch,order);
    models[M_DISTANCE]->p(*m);
    models[M_INDIRECT]->p(*m);
    models[M_DMC]->p(*m);
    models[M_DEC]->p(*m);
    m->set(order << 3U | x.bpos, 64);
    if (slow==true) models[M_CHART]->p(*m);
    if (slow==true) models[M_LSTM]->p(*m);
    U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
    m->set(8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512, 8+1024);
    m->set(x.c0, 256);
    m->set(order+8*(x.c4>>6&3)+32*(x.bpos==0)+64*(c1==c2)| (1)<<7, 256);  
    m->set(c2, 256);
    m->set(c3, 256);
    if (x.bpos) {
      c=x.c0<<(8-x.bpos); if (x.bpos==1)c+=c3/2;
      c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
    }
    else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
    m->set(c, 1536);
    pr0=m->p(0,1); //0,1
    int const limit = 0x3FFu >> ((x.blpos < 0xFFFu) * 4);
    pr = DEC.APMs[0].p(pr0, (x.DEC.state * 26u) + x.DEC.bcount, x.y,limit);
    pr = (pr * 4 + pr0 * 2 + 3) / 6;
    pr=(4096-pr)*(32768/4096);
    if(pr<1) pr=1;
    if(pr>32767) pr=32767;
}
