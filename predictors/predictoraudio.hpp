#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
// Audio predicor
class PredictorAUDIO2: public Predictors {
    int pr;
    Mixers *m;
    EAPM a;
    eSSE sse;
    const std::vector<ModelTypes> activeModels {
        M_RECORD,
        M_MATCH,    
        M_WAV};
    void setmixer();
public:
    int mcxt[1];
    int p()  const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    ~PredictorAUDIO2(){ }
    PredictorAUDIO2(Settings &set);
    void update();
};
