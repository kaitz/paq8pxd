#include "predictorexe.hpp"

// x86/64 predicor
PredictorEXE::PredictorEXE(Settings &set):Predictors(set), pr(16384),order(0),
    x86_64{
        { /*APM:*/ {0x800,20}, {0x10000,16}, {0x10000,16}/*, {0x10000,16}, {0x10000,16}*/ }
    },
    count(0), sse(x) {

    loadModels(activeModels);
    // add extra 
    mixerInputs+=1;
    sse.p(pr);

    // Predictor contexts
    mxp.push_back( {8+1024,64,0,28,&mcxt[0],0,false} );
    mxp.push_back( {   256,64,0,28,&mcxt[1],0,false} );
    mxp.push_back( {   256,64,0,28,&mcxt[2],0,false} );
    mxp.push_back( {   256,64,0,28,&mcxt[3],0,false} );
    mxp.push_back( {   256,64,0,28,&mcxt[4],0,false} );
    mxp.push_back( {   256,64,0,28,&mcxt[5],0,false} );
    mxp.push_back( {  1536,64,0,28,&mcxt[6],0,false} );
    mxp.push_back( {     1, 8,7,14,&mcxt[7],0,false} ); // final mixer
    // create mixer
    m=new Mixers(x,mxp.size(),mixerInputs,mxp);
    mcxt[7]=0;
    einfo.reset();
}

void PredictorEXE::update()  {
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    if (einfo.stat(pr,x.y)) {
        const int el=(14-einfo.rates)*16;
        m->setErrLimit(el,einfo.rates*2);
    }
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
    models[M_SPARSE_Y]->p(*m,ismatch,order);
    models[M_DISTANCE]->p(*m);
    models[M_INDIRECT]->p(*m);
    models[M_EXE] ->p(*m,1);
    if (x.settings.slow==false) models[M_XML]->p(*m);
    if (x.settings.slow==true) models[M_PPM]->p(*m); 
    if (x.settings.slow==true) models[M_CHART]->p(*m);
    if (x.settings.slow==true) models[M_LSTM]->p(*m);
    U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
    mcxt[0]=8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512;
    mcxt[1]=x.c0;
    mcxt[2]=c2;
    U8 d=x.c0<<(8-x.bpos);
    mcxt[3]=order+8*(x.c4>>6&3)+32*(x.bpos==0)+64*(c1==c2)+1*128;
    mcxt[4]=c3;
    mcxt[5]=ismatch;
    if (x.bpos) {
        c=d; if (x.bpos==1)c+=c3/2;
        c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
    }
    else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
    mcxt[6]=c;
    int pr0=m->p(2,1);
    int const limit = 0x3FFu >> (static_cast<int>(x.blpos < 0xFFFu) * 4);
    pr = x86_64.APMs[0].p(pr0, (x.x86_64.state << 3u) | x.bpos,x.y, limit);
    int  pr1 = x86_64.APMs[1].p(pr0, (x.c0 << 8u) | x.x86_64.state,x.y, limit);
    int  pr2 = x86_64.APMs[2].p(pr0, finalize64(hash(x.c4 & 0xFFu, x.bpos,x.x86_64.flags, x.x86_64.state >> 3), 16),x.y, limit);
    pr = (pr + pr0 + pr1 + pr2 + 2) >> 2;
    //int pr3 = x86_64.APMs[3].p(pr, hash(x.c0, x.Match.byte, x.x86_64.flags)&0xFFFF, 5);
    //int pr4 = x86_64.APMs[4].p(pr, hash(x.c0, x.x86_64.code, x.c4&0xffff)&0xFFFF, 6);
    //pr = (pr+pr3+2)>>1;
    sse.update();
    pr = sse.p(pr);
    /*pr=(4096-pr)*(32768/4096);
    if(pr<1) pr=1;
    if(pr>32767) pr=32767;*/
}
