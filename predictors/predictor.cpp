#include "predictor.hpp"

// General predicor class

extern bool slow;

Predictor::Predictor(): pr(16384),pr0(pr),order(0),ismatch(0), a(x),isCompressed(false),count(0),lastmiss(0),
 sse(x){
   loadModels(activeModels,18);
   // add extra 
   mixerInputs+=1;
   mixerNets+=64+    (8+1024)+    256+    512+   2048+   2048+    256+    1536;
   mixerNetsCount+=8;
   sse.p(pr);
   // create mixer
   m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount);
}

void Predictor::update()  {
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    x.Misses+=x.Misses+((pr>>11)!=x.y);
    
    if (x.bpos==0) {
        lastmiss=x.Misses&0xFF?x.blpos:lastmiss;
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
        if(x.blpos==1) {
            isCompressed=(x.filetype==CMP || x.filetype==MSZIP)?true:false;
           
         }
        /*if(x.filetype==BINTEXT) m->setl(3,2);
        else*/ m->setl(7,1);
    }
    m->update();
    m->add(256);
    int rlen=0,Valid=0,xmlstate=0;
    ismatch=models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    if (x.bpos==0)x.count++;
    models[M_SPARSEMATCH]->p(*m);
    order=models[M_NORMAL]->p(*m);
    if ( isCompressed==false){        
            int dataRecordLen=(x.filetype==DBASE)?x.finfo:(x.filetype==IMGUNK)?x.finfo:0; //force record length 
            rlen=models[M_RECORD]->p(*m,dataRecordLen);
            models[M_WORD]->p(*m,0,x.finfo>0?x.finfo:0); //col
            models[M_SPARSE_Y]->p(*m,ismatch,order);
            models[M_DISTANCE]->p(*m);
            models[M_INDIRECT]->p(*m);
            models[M_NEST]->p(*m);
            models[M_DMC]->p(*m);
            if (slow==true) models[M_PPM]->p(*m); 
            if (slow==true) models[M_CHART]->p(*m);
            if (slow==true) models[M_LSTM]->p(*m);
            xmlstate=models[M_XML]->p(*m);
            models[M_TEXT]->p(*m);
            Valid=models[M_EXE]->p(*m);
            
            if (!(x.filetype==DBASE/*||x.filetype==BINTEXT*/||x.filetype==HDR ||x.inpdf==false )) models[M_LINEAR]->p(*m);
    } 
    m->set((order<<3)|x.bpos, 64);
    U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
    m->set(8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512, 8+1024);
    m->set(x.c0, 256);
    uint32_t bt = x.filetype==DEFAULT ? 0 : (/*x.filetype==BINTEXT ||*/ x.filetype==DBASE) ? 2 : Valid ? 1 : 3;
    m->set(order+8*(x.c4>>6&3)+32*(x.bpos==0)+64*(c1==c2)+128*bt, 512);
    U8 d=x.c0<<(8-x.bpos);
    m->set(((xmlstate&3)>0)*1024+(x.bpos>0)*512+(order>3)*256+(x.w4&240)+(x.b3>>4),2048);
    m->set(x.bpos*256+((x.words<<x.bpos&255)>>x.bpos|(d&255)),2048);
    m->set(ismatch, 256);
    if (x.bpos) {
      c=d; if (x.bpos==1)c+=c3/2;
      c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
    }
    else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
    m->set(c, 1536);
    pr0=m->p(1,1);
    pr=a.p2(pr0,pr,7);
    sse.update();
    pr = sse.p(pr);
}

