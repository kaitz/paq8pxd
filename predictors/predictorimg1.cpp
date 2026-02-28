#include "predictorimg1.hpp"
// 1-bit image predicor

PredictorIMG1::PredictorIMG1(Settings &set):Predictors(set), pr(16384), sse(x) {
    loadModels(activeModels);  
    // add extra 
    mixerInputs+=1;
    sse.p(pr);

    for (int i=0; i<2; i++) mcxt[i]=0;
    mxp.push_back( {  256,55,7,24,&mcxt[0],0} );
    mxp.push_back( {1,6,7,4,&mcxt[1],0} ); // final mixer
    // create mixer
    m=new Mixers(x,mxp.size(),mixerInputs,mxp);
}

void PredictorIMG1::update()  {
    m->update();
    m->add(256); 
    int ismatch=models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    if (x.settings.slow==true) models[M_LSTM]->p(*m);
    mcxt[0]=ismatch;
    models[M_IM1]->p(*m, x.finfo);
    pr=m->p(); 
    sse.update();
    pr = sse.p(pr);
}

