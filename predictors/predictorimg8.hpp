#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"

// 8-bit image predicor
class PredictorIMG8: public Predictors {
    int pr;
    Mixers *m;
    ErrorInfo einfo;
    struct {
        struct {
            APM APMs[4];
            APM1 APM1s[2];
        } Palette;
        struct {
            APM APMs[3];
        } Gray;
    } Image;
    StateMap StateMaps[2];
    eSSE sse;
    const std::vector<ModelTypes> activeModels { 
        M_MATCH,
        M_IM8,
        M_LSTM};
public:
    int mcxt[1];
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    ~PredictorIMG8() {
        //printf("IM8 mixer inputs: %d\n",m->tx.size());
        delete m;
    }
    PredictorIMG8(Settings &set);
    void update();
};
