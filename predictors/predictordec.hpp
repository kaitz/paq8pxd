#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"

// DECAlpha predicor
class PredictorDEC: public Predictors {
    int pr;
    int pr0;
    int order;
    int ismatch;
    Mixers *m; 
    struct {
        APM APMs[1];
    } DEC;
    eSSE sse;
    const std::vector<MType> activeModels { 
        {M_RECORD,      SLOW|EXTREME},
        {M_MATCH,       SLOW|EXTREME},
        {M_MATCH1,      SLOW|EXTREME},
        {M_DISTANCE,    SLOW|EXTREME},
        {M_INDIRECT,    SLOW|EXTREME},
        {M_DMC,         SLOW|EXTREME},
        {M_NEST,        SLOW|EXTREME},
        {M_NORMAL,      SLOW|EXTREME},
        {M_TEXT,        SLOW|EXTREME},
        {M_WORD,        SLOW|EXTREME},
        {M_DEC,         SLOW|EXTREME}, 
        {M_SPARSEMATCH, SLOW|EXTREME},
        {M_SPARSE_Y,    SLOW|EXTREME},
        {M_CHART,            EXTREME},
        {M_LSTM,             EXTREME}
    };
public:
    int mcxt[8];
    PredictorDEC(Settings &set);
    int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    void update() ;
    ~PredictorDEC() {
        delete m;
    }
};

