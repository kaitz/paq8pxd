#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"

// x86/64 predicor
class PredictorEXE: public Predictors {
    int pr;  // next prediction
    int order;
    Mixers *m;
    struct {
        APM APMs[3];
    } x86_64;
    U32 count;
    eSSE sse;
    const std::vector<ModelTypes> activeModels { 
        M_RECORD,
        M_MATCH ,
        M_MATCH1, 
        M_DISTANCE, 
        M_EXE, 
        M_INDIRECT, 
        M_DMC, 
        M_NEST, 
        M_NORMAL, 
        M_XML, 
        M_TEXT, 
        M_WORD,
        M_SPARSEMATCH, 
        M_SPARSE_Y,
        M_PPM,M_CHART,M_LSTM};
public:
    int mcxt[8];
    PredictorEXE(Settings &set);
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    void update() ;
    ~PredictorEXE() {
        delete m;
    }
};


