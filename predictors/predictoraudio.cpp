#include "predictoraudio.hpp"
// Audio predicor

PredictorAUDIO2::PredictorAUDIO2(Settings &set):Predictors(set), pr(16384),a(x), sse(x) {
    loadModels(activeModels);  
    // add extra 
    mixerInputs+=1;
    sse.p(pr);

    mxp.push_back( {1,8,0,14,&mcxt[0],0} ); // final mixer
    // create mixer
    m=new Mixers(x,mxp.size(),mixerInputs,mxp);
    mcxt[0]=0;
}

void PredictorAUDIO2::update()  {
    m->update();
    m->add(256);
    models[M_MATCH]->p(*m);  
    models[M_RECORD]->p(*m);
    models[M_WAV]->p(*m,x.finfo);
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    pr=a.p1(m->p(((x.finfo&2)==0),1),pr,7);
    sse.update();
    pr = sse.p(pr);
}

