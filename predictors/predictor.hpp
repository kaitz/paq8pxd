#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
#include "../prt/settings.hpp"

// General predicor class
class Predictor: public Predictors {
    int pr;  // next prediction
    int pr0;
    int order;
    int ismatch;
    Mixers *m;
    ErrorInfo einfo;
    EAPM a;
    bool isCompressed; 
    U32 count;
    U32 lastmiss;
    eSSE sse;
    const std::vector<ModelTypes> activeModels { 
        M_RECORD,
        M_MATCH ,
        M_MATCH1, 
        M_DISTANCE, 
       // M_EXE, 
        M_INDIRECT, 
        M_DMC, 
        M_NEST, 
        M_NORMAL, 
        M_XML, 
        M_TEXT, 
        M_WORD ,
        M_LINEAR, 
        M_SPARSEMATCH, 
        M_SPARSE_Y,
        M_PPM,M_CHART,M_LSTM};

public:
    int mcxt[9];
    Predictor(Settings &);
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    void update();
    ~Predictor() {
        delete m;
    }
};
