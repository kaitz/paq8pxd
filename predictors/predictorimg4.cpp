#include "predictorimg4.hpp"

// 4-bit image predicor
PredictorIMG4::PredictorIMG4(Settings &set):Predictors(set), pr(16384), StateMaps{ 16, 256, 256},
    Image{{{0x1000,20}, {0x10000,20}, {0x10000,20}, {0x10000}},  {{0x10000,x}, {0x10000,x}}},
    sse(x) {
    
    loadModels(activeModels);  
    // add extra 
    mixerInputs+=1+3;
    sse.p(pr);
    //for (int i=0;i<1;i++) mcxt[i]=0;

    mxp.push_back( {1,8,7,14,&mcxt[0],0,false} ); // final mixer
    // create mixer
    m=new Mixers(x,mxp.size(),mixerInputs,mxp);
    mcxt[0]=0;
}

void PredictorIMG4::update()  {
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    x.Misses+=x.Misses+((pr>>11)!=x.y); // ???
    m->update();
    m->add(256);
    if (einfo.stat(pr,x.y,1)) {
        const int el=(14-einfo.rates)*16;
        m->setErrLimit(el); // no lr
    }
    models[M_MATCH]->p(*m); // byte level, my be bad
    models[M_IM4]->p(*m,x.finfo);
    if (x.settings.slow==true) models[M_LSTM]->p(*m);
    m->add((stretch(StateMaps[0].p(x.Image.pixels.px,x.y))+1));
    m->add((stretch(StateMaps[1].p(x.Image.pixels.px|(x.Image.pixels.W<<4),x.y))+1));
    m->add((stretch(StateMaps[2].p(x.Image.pixels.px|(x.Image.pixels.N<<4),x.y))+1));
    int pr0=m->p(2+min(1,m->nx/(m->zpr+1)),1);
    int pr1, pr2, pr3;
    int limit=0x3FF;//>>((x.blpos<0xFFF)*4);
    pr  = Image.APMs[0].p(pr0, (x.Image.pixels.px<<4)|(x.Misses&0xF), x.y, limit);
    pr1 = Image.APMs[1].p(pr0, (x.Image.pixels.px*16+ x.Image.pixels.W)*256+ x.Image.pixels.N*16+(x.Misses&0xF),x.y, limit);
    pr2 = Image.APMs[2].p(pr0, (x.Image.pixels.px*16+ x.Image.pixels.N)*256+ x.Image.pixels.NN*16+(x.Misses&0xF),x.y, limit);
    pr3 = Image.APMs[3].p(pr0, (x.Image.pixels.px*16+ x.Image.pixels.W)*256+ x.Image.pixels.WW*16+(x.Misses&0xF),x.y, limit);
    pr0 = (pr0+pr1+pr2+pr3+2)>>2;

    pr1 = Image.APM1s[0].p(pr0, x.Image.pixels.px+ x.Match.byte*256+ x.Image.pixels.N*16, 5);
    pr2 = Image.APM1s[1].p(pr, x.Image.pixels.px+ x.Image.pixels.W*256+ x.Image.pixels.N*16, 6);
    pr = (pr*2+pr1+pr2+2)>>2;
    pr = (pr+pr0*3+1)>>2;
    sse.update();
    pr = sse.p(pr);
     /*pr=(4096-pr)*(32768/4096);
    if(pr<1) pr=1;
    if(pr>32767) pr=32767;*/
}

