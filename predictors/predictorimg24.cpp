#include "predictorimg24.hpp"

// 24/32-bit image predicor
PredictorIMG24::PredictorIMG24(Settings &set):Predictors(set), pr(16384),
    Image{ {0x1000/*, 0x10000, 0x10000, 0x10000*/}, {{0x10000,x}, {0x10000,x}} },
    StateMaps{ 256, 256*256}, sse(x){

    loadModels(activeModels);   
    // add extra 
    mixerInputs+=1+2;
    sse.p(pr);

    //for (int i=0; i<2; i++) mcxt[i]=0;
    mxp.push_back( {  8192,64,0,28,&mcxt[0],0} );
    mxp.push_back( {1,8,0,14,&mcxt[1],0} ); // final mixer
    // create mixer
    m=new Mixers(x,mxp.size(),mixerInputs,mxp);
    mcxt[1]=0;
}

void PredictorIMG24::update() {
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;

    x.Misses+=x.Misses+((pr>>11)!=x.y);
    m->update();
    m->add(256);
    if (x.bpos==0)x.count++;
    models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    //if (slow==true) models[M_NORMAL]->p(*m);
    if (x.settings.slow==true) models[M_LSTM]->p(*m);
    models[M_IM24]->p(*m,x.finfo);
    m->add((stretch(StateMaps[0].p(x.c0,x.y))+1)>>1);
    m->add((stretch(StateMaps[1].p(x.c0|(x.buf(1)<<8),x.y))+1)>>1);
    mcxt[0]=x.Image.ctx&0x1FFF;
    int pr1, pr2, pr3;
    int pr0=x.filetype==IMAGE24? m->p(1,1): m->p();
    int limit=0x3FF>>((x.blpos<0xFFF)*4);
    // pr=pr0;
    pr  = Image.APMs[0].p(pr0, (x.c0<<4)|(x.Misses&0xF), x.y,limit);
    /* pr1 = Image.APMs[1].p(pr0, hash(x.c0, x.Image.pixels.W, x.Image.pixels.WW)&0xFFFF,  x.y,limit);
pr2 = Image.APMs[2].p(pr0, hash(x.c0, x.Image.pixels.N, x.Image.pixels.NN)&0xFFFF, x.y, limit);
pr3 = Image.APMs[3].p(pr0, (x.c0<<8)|x.Image.ctx, x.y, limit);
pr0 = (pr0+pr1+pr2+pr3+2)>>2;
*/
    pr1 = Image.APM1s[0].p(pr, hash(x.c0, x.Image.pixels.W, x.buf(1)-x.Image.pixels.Wp1, x.Image.plane)&0xFFFF);
    pr2 = Image.APM1s[1].p(pr, hash(x.c0, x.Image.pixels.N, x.buf(1)-x.Image.pixels.Np1, x.Image.plane)&0xFFFF);
    pr=(pr*2+pr1*3+pr2*3+4)>>3;
    pr = (pr+pr0+1)>>1;
    sse.update();
    pr = sse.p(pr);
}

