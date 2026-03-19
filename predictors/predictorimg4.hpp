#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"

// 4-bit image predicor
class PredictorIMG4: public Predictors {
    int pr;
    Mixers *m;
    ErrorInfo einfo;
    StateMap StateMaps[3];
    struct {
        APM APMs[4];
        APM1 APM1s[2];
    } Image;
    eSSE sse;
    const std::vector<ModelTypes> activeModels { 
        M_MATCH ,
        M_IM4,
        M_LSTM};
public:
    int mcxt[1];
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    ~PredictorIMG4() {
        //printf("IM4 mixer inputs: %d\n",m->tx.size());
        delete m;
    }
    PredictorIMG4(Settings &set);
    void update();
};
