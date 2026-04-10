#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/ESSE.hpp"

// 1-bit image predicor
class PredictorIMG1: public Predictors {
    int pr;  // next prediction
    Mixers *m;
    ErrorInfo einfo;
    eSSE sse;
    APM apm;
    const std::vector<MType> activeModels { 
        {M_MATCH, SLOW|EXTREME},
        {M_IM1,   SLOW|EXTREME},
        {M_LSTM,       EXTREME}
    };
public:
    int mcxt[2];
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    ~PredictorIMG1() {
        //printf("IM1 mixer inputs: %d\n",m->tx.size());
        delete m;
    }
    PredictorIMG1(Settings &set);
    void update();
};
