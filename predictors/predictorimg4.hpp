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
    const std::vector<MType> activeModels { 
        {M_MATCH, SLOW|EXTREME},
        {M_IM4,   SLOW|EXTREME},
        {M_LSTM,       EXTREME}
    };
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
