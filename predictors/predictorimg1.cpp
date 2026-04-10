#include "predictorimg1.hpp"
// 1-bit image predicor

PredictorIMG1::PredictorIMG1(Settings &set):Predictors(set), pr(16384), sse(x),apm(0x10000) {
    loadModels(activeModels);  
    // add extra 
    mixerInputs+=1;
    sse.p(pr);

    mxp.push_back( {  256,64,0,28,&mcxt[0],0,false} );
    mxp.push_back( {    1, 8,0,14,&mcxt[1],0,false} ); // final mixer
    // create mixer
    m=new Mixers(x,mxp.size(),mixerInputs,mxp);
    mcxt[1]=0; // Enable final mixer
    einfo.reset();
}

void PredictorIMG1::update()  {
    m->update();
    m->add(256); 
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    if (einfo.stat(pr,x.y)) {
        const int el=(14-einfo.rates)*8;
        m->setErrLimit(el); // slow, range 14...2
    }
    int ismatch=models[M_MATCH]->p(*m);
    models[M_LSTM]->p(*m);
    mcxt[0]=ismatch;
    models[M_IM1]->p(*m, x.finfo);
    pr=m->p(3,1);
    //pr=pr*3+apm.p(pr, 32*x.y^hash(s4,ismatch,x.Misses&0xf)&0xffff, x.y,5)>>2;
    //sse.update();
    //pr = sse.p(pr);
    pr=(4096-pr)*(32768/4096);
    if(pr<1) pr=1;
    if(pr>32767) pr=32767;
}

