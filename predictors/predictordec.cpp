#include "predictordec.hpp"
// DECAlpha predicor

PredictorDEC::PredictorDEC(Settings &set):Predictors(set),
    pr(16384),pr0(pr),order(0),ismatch(0),
    DEC{
       { /*APM:*/ { 25*26,20} }
    },
    sse(x) {

    loadModels(activeModels);
    // add extra 
    mixerInputs+=1;
    sse.p(pr);
    // create mixer
    for (int i=0;i<8;i++) mcxt[i]=0;

    // Predictor contexts
    mxp.push_back( {8+1024,55,7,24,&mcxt[0],0} );
    mxp.push_back( {   256,55,7,24,&mcxt[1],0} );
    mxp.push_back( {   256,55,7,24,&mcxt[2],0} );
    mxp.push_back( {   256,55,7,24,&mcxt[3],0} );
    mxp.push_back( {   256,55,7,24,&mcxt[4],0} );
    mxp.push_back( {   1536,55,7,24,&mcxt[5],0} );
    mxp.push_back( {   64,55,7,24,&mcxt[6],0} );
    
    mxp.push_back( {1,6,7,4,&mcxt[7],0} ); // final mixer
    // create mixer
    m=new Mixers(x,mxp.size(),mixerInputs,mxp);
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
    mcxt[6]=order << 3U | x.bpos;
    if (x.settings.slow==true) models[M_CHART]->p(*m);
    if (x.settings.slow==true) models[M_LSTM]->p(*m);
    U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
    mcxt[0]=8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512;
    mcxt[1]=x.c0;
    mcxt[2]=order+8*(x.c4>>6&3)+32*(x.bpos==0)+64*(c1==c2)| (1)<<7;  
    mcxt[3]=c2;
    mcxt[4]=c3;
    if (x.bpos) {
        c=x.c0<<(8-x.bpos); if (x.bpos==1)c+=c3/2;
        c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
    }
    else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
    mcxt[5]=c;
    pr0=m->p(); //0,1
    int const limit = 0x3FFu >> ((x.blpos < 0xFFFu) * 4);
    pr = DEC.APMs[0].p(pr0, (x.DEC.state * 26u) + x.DEC.bcount, x.y,limit);
    pr = (pr * 4 + pr0 * 2 + 3) / 6;
    pr=(4096-pr)*(32768/4096);
    if(pr<1) pr=1;
    if(pr>32767) pr=32767;
}
