#include "predictorimg1.hpp"
// 1-bit image predicor

PredictorIMG1::PredictorIMG1(Settings &set):Predictors(set), pr(16384), sse(x),apm(0x10000) {
    loadModels(activeModels);  
    // add extra 
    mixerInputs+=1;
    sse.p(pr);

    //for (int i=0; i<2; i++) mcxt[i]=0;
    mxp.push_back( {  256,64,0,28,&mcxt[0],0} );
    mxp.push_back( {    1, 8,0,14,&mcxt[1],0} ); // final mixer
    // create mixer
    m=new Mixers(x,mxp.size(),mixerInputs,mxp);
    //for (size_t i=0;i<mxp.size();i++)  *mxp[i].cxt=-1;
    mcxt[1]=0; // Enable final mixer
}

void PredictorIMG1::update()  {
    m->update();
    m->add(256); 
    int ismatch=models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    if (x.settings.slow==true) models[M_LSTM]->p(*m);
    mcxt[0]=ismatch;
    int s4=models[M_IM1]->p(*m, x.finfo);
    pr=m->p();
    //pr=pr*3+apm.p(pr, s4+ismatch, x.y,3)>>2;
    //sse.update();
    //pr = sse.p(pr);
     pr=(4096-pr)*(32768/4096);
    if(pr<1) pr=1;
    if(pr>32767) pr=32767;
}

