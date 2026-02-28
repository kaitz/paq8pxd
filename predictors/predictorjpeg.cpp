#include "predictorjpeg.hpp"
// Jpeg predicor

PredictorJPEG::PredictorJPEG(Settings &set):Predictors(set), pr(16384), 
  Jpeg{
    { /*APM:*/ { 0x2000} }
  },
  Bypass(false){
   loadModels(activeModels);
   // add extra 
   mixerInputs+=3+1+1-3;
   mixerNets+=        (8+1024)+       256+       256+       256+       256+       1536;
     
   mixerNetsCount+=6;
   // create mixer
   //m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount,0,3,2); //set  error update rate to 1.5 (3/2), default is 7/1
   for (int i=0;i<9;i++) mcxt[i]=0;
   
    // Predictor contexts - select larger cxt     //def    jpg
    mxp.push_back( {8+1024,55,7,24,&mcxt[0],0} ); //8+1024    9
    mxp.push_back( {  1025,55,7,24,&mcxt[1],0} ); //256    1025
    mxp.push_back( {  1024,55,7,24,&mcxt[2],0} ); //256    1024
    mxp.push_back( {   512,55,7,24,&mcxt[3],0} ); //256     512
    mxp.push_back( {  4096,55,7,24,&mcxt[4],0} ); //256    4096
    mxp.push_back( {  1536,55,7,24,&mcxt[5],0} ); //1536     64
    mxp.push_back( {  4096,55,7,24,&mcxt[6],0} ); //0      4096
    mxp.push_back( {  1024,55,7,24,&mcxt[7],0} ); //0      1024
    

   mxp.push_back( {1,6,7,4,&mcxt[8],0} ); // final mixer
   // create mixer
   m=new Mixers(x,mxp.size(),mixerInputs,mxp);
}

void PredictorJPEG::update()  {
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    x.Misses+=x.Misses+((pr>>10)!=x.y);
    m->update();
    m->add(256);
    Bypass=false;
    int ismatch=models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    if (x.settings.slow==true) models[M_LSTM]->p(*m);
    if (x.settings.slow==false && (x.Match.length>0xFF || x.Match.bypass)) {//256b
        x.Match.bypass =   Bypass =    true;
        pr= x.Match.bypassprediction;
        pr=(4096-pr)*(32768/4096);
        if(pr<1) pr=1;
        if(pr>32767) pr=32767;
        models[M_JPEG]->p(*m,1);//we found long repeating match. update, do not predict. artificial images, same partial content
        //m->reset(); 
        return;
    }
    if (models[M_JPEG]->p(*m)) {
        for (int i=0;i<8;i++) mcxt[i]=dynamic_cast<jpegModelx *>(models[M_JPEG])->mc(i);
        if (x.settings.slow==true) models[M_NORMAL]->p(*m);
        pr=m->p();
    }
    else{
        int order =models[M_NORMAL]->p(*m);
        U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
        mcxt[0]=8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512;
        mcxt[1]=x.c0;
        mcxt[2]=order | ((x.c4>>6)&3)<<3 | (x.bpos==0)<<5 | (c1==c2)<<6 | (1)<<7;
        mcxt[3]=c2;
        mcxt[4]=c3;
        U8 d=x.c0<<(8-x.bpos);
        if (x.bpos) {
            c=d; if (x.bpos==1)c+=c3/2;
            c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
        }
        else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
        mcxt[5]=c;
        pr=m->p();
    }
    U32 pr0 = Jpeg.APMs[0].p(pr , x.JPEG.state,x.y, 0x3FF);
    pr = (pr + pr0 + 1) / 2;
    pr=(4096-pr)*(32768/4096);
    if(pr<1) pr=1;
    if(pr>32767) pr=32767;
}

