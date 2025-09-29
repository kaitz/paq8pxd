#include "predictors.hpp"
//////////////////////////// Predictor /////////////////////////
// A Predictor estimates the probability that the next bit of
// uncompressed data is 1.  Methods:
// p() returns P(1) as a 12 bit number (0-4095).
// update(y) trains the predictor with the actual bit (0 or 1).

extern bool slow;

Predictors::Predictors():  mixerInputs(0),mixerNets(0),mixerNetsCount(0){
}
  void Predictors::loadModels(const U8* amodel,int count){
      models = new Model*[M_MODEL_COUNT];
      // reset
      for (int i=0;i<M_MODEL_COUNT;i++)      models[i]=0;
      // create active models
      for (int i=0;i<count;i++){
#ifdef VERBOSE      
          printf("Creating %s\n",modelNames[amodel[i]].c_str());
#endif
      switch (amodel[i]){
          case M_RECORD:{
              models[M_RECORD] =      new recordModel1(x);
              break;
          }
          case M_MATCH:{
              models[M_MATCH] =       new matchModel1(x);
              break;
          }
          case M_MATCH1:{
              models[M_MATCH1] =      new matchModel2(x);
              break;
          }
          case M_DISTANCE: {
              models[M_DISTANCE] =    new distanceModel1(x);
              break;
          }
          case M_EXE:{
              models[M_EXE] =         new exeModel1(x);
              break;
          }
          case M_INDIRECT:{
              models[M_INDIRECT] =    new indirectModel1(x);
              break;
          }
          case M_DMC:{
              models[M_DMC] =         new dmcModel1(x);
              break;
          } 
          case M_NEST:{
              models[M_NEST] =        new nestModel1(x);
              break;
          }
          case M_NORMAL:{
              models[M_NORMAL] =      new normalModel1(x);
              break;
          }
          case M_XML:{
              models[M_XML] =         new XMLModel1(x);
              break;
          } 
          case M_TEXT:{
              models[M_TEXT] =        new TextModel(x,16);
              break;
          }
          case M_WORD:{
              models[M_WORD] =        new wordModel1(x);
              break;
          } 
          case M_LINEAR:{
              models[M_LINEAR] =      new linearPredictionModel(x);
              break;
          }
          case M_SPARSEMATCH:{
              models[M_SPARSEMATCH] = new SparseMatchModel(x);
              break;
          }
          case M_SPARSE_Y:{
              models[M_SPARSE_Y] =    new sparseModely(x);
              break;
          }
          case M_DEC:{
              models[M_DEC] =         new decModel1(x);
              break;
          }
          case M_IM8:{
              models[M_IM8] =         new im8bitModel1(x);
              break;
          }
          case M_IM24:{
              models[M_IM24] =        new im24bitModel1(x);
              break;
          }
          case M_SPARSE:{
              models[M_SPARSE] =      new sparseModelx(x);
              break;
          }
          case M_JPEG:{
              models[M_JPEG] =        new jpegModelx(x);
              break;
          }
          case M_WAV:{
              models[M_WAV] =         new wavModel1(x);
              break;
          }
          case M_IM4:{
              models[M_IM4] =         new im4bitModel1(x);
              break;
          }
          case M_IM1:{
              models[M_IM1] =         new im1bitModel1(x);
              break;
          }
          case M_PPM:{
              if (slow==true)
              models[M_PPM] =         new ppmdModel1(x);
              else
              models[M_PPM] =         new blankModel1(x); 
              break;
          }
          case M_CHART:{
              if (slow==true)
              models[M_CHART] =       new chartModel(x);
              else
              models[M_CHART] =       new blankModel1(x);
              break;
          }
          case M_LSTM:{
              if (slow==true)
              models[M_LSTM] =        new lstmModel1(x);
              else
              models[M_LSTM] =        new blankModel1(x);
              break;
          }
          default:{
              quit("Error: wrong model.");
              break;
          }
     }
  }
  // create blank models
   for (int i=0;i<M_MODEL_COUNT;i++){
       if (models[i] ==0) models[i] =  new blankModel1(x);
  }
  // get mixer data from models
   for (int i=0;i<M_MODEL_COUNT;i++){
       mixerInputs+=models[i]->inputs();
       mixerNets+=models[i]->nets();
       mixerNetsCount+=models[i]->netcount();
   }
  }
  void Predictors::setContexts(){
    const U8 c1=x.buf(1),c2=x.buf(2),c3=x.buf(3);
    if((c2=='.'||c2=='!'||c2=='?' ||c2=='}') && !(c3==10 || c3==5) && 
    (x.filetype==DICTTXT ||x.filetype==TEXT0|| x.filetype==BIGTEXT)) for (int i=14; i>0; --i) 
      x.cxt[i]=x.cxt[i-1]*primes[i];
    x.cxt[15]=(isalpha(c1))?(x.cxt[15]*primes[15]+ tolower(c1)):0;
    for (int i=14; i>0; --i)  // update order 0-11 context hashes
      //cxt[i]=cxt[i-1]*primes[i]+(x.c4&255)+1;
      x.cxt[i] = hash(x.cxt[i - 1], c1+ (i << 10));
  }
  void Predictors::update0(){
    // Update global context: pos, bpos, c0, c4, buf
    // called from encoder
    x.c0+=x.c0+x.y;
    if (x.c0>=256) {
        x.buf[x.buf.pos++]=x.c0;
        x.c0=1;
        ++x.blpos;
        x.buf.pos=x.buf.pos&x.buf.poswr; //wrap
        x.c4=(x.c4<<8)|x.buf(1);
        x.c8=(x.c8<<8)|x.buf(5);
        setContexts();
    }
    x.bpos=(x.bpos+1)&7;
    x.grp = (x.bpos>0)?AsciiGroupC0[(1<<x.bpos)-2+(x.c0&((1<<x.bpos)-1))]:0;
  }


