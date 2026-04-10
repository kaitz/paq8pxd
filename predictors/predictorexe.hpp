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
    ErrorInfo einfo;
    struct {
        APM APMs[3];
    } x86_64;
    U32 count;
    eSSE sse;
    const std::vector<MType> activeModels { 
        {M_RECORD,      SLOW|EXTREME},
        {M_MATCH,       SLOW|EXTREME},
        {M_MATCH1,      SLOW|EXTREME},
        {M_DISTANCE,    SLOW|EXTREME},
        {M_EXE,         SLOW|EXTREME},
        {M_INDIRECT,    SLOW|EXTREME},
        {M_NORMAL,      SLOW|EXTREME},
        {M_XML,         SLOW},
        {M_SPARSEMATCH, SLOW|EXTREME},
        {M_SPARSE_Y,    SLOW|EXTREME},
        {M_PPM,              EXTREME},
        {M_CHART,            EXTREME},
        {M_LSTM,             EXTREME}
    };
public:
    int mcxt[8];
    PredictorEXE(Settings &set);
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    void update() ;
    ~PredictorEXE() {
        delete m;
    }
};


