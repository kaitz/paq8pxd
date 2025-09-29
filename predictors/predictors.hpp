#pragma once
#include "../prt/enums.hpp"
#include "../models/model.hpp"
#include "../models/sparsematch.hpp"
#include "../models/exe.hpp"
#include "../models/dec.hpp"
#include "../models/indirect.hpp"
#include "../models/record.hpp"
#include "../models/dmc.hpp"
#include "../models/sparsex.hpp"
#include "../models/jpeg.hpp"
#include "../models/match1.hpp"
#include "../models/match2.hpp"
#include "../models/normal.hpp"
#include "../models/chart.hpp"
#include "../models/xml.hpp"
#include "../models/nested.hpp"
#include "../models/audio.hpp"
#include "../models/linear.hpp"
#include "../models/im1bit.hpp"
#include "../models/sparsey.hpp"
#include "../models/distance.hpp"
#include "../models/im4bit.hpp"
#include "../models/im8bit.hpp"
#include "../models/im24bit.hpp"
#include "../models/blank.hpp"
#include "../models/lstm.hpp"
#include "../models/word.hpp"
#include "../models/text.hpp"
#include "../models/ppmd.hpp"
//////////////////////////// Predictor /////////////////////////
// A Predictor estimates the probability that the next bit of
// uncompressed data is 1.  Methods:
// p() returns P(1) as a 12 bit number (0-4095).
// update(y) trains the predictor with the actual bit (0 or 1).

//base class
class Predictors {
public:
  BlockData x; //maintains current global data block between models
  int mixerInputs,mixerNets,mixerNetsCount;
  Model **models;

virtual ~Predictors(){
    //printf("Models peak memory %d mb\n",(getPeakMemory()/1024)/1024);
    for (int i=0;i<M_MODEL_COUNT;i++) {
        delete models[i];
    }
    delete[] models;    
   };
Predictors();
  virtual int p() const =0;
  virtual void update()=0;
  void loadModels(const U8* amodel,int count);
  void setContexts();
  void update0();
};

