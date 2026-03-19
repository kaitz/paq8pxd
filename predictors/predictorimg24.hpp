#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"
#include "../prt/APMPost.hpp"

// 24/32-bit image predicor
class PredictorIMG24: public Predictors {
    int pr;  // next prediction
    Mixers *m;
    ErrorInfo einfo;
    struct {
        APM APMs[1];
        APM1 APM1s[2];
    } Image;
    APMPost APMPostA, APMPostB;
    StateMap StateMaps[2];
    eSSE sse;
    const std::vector<ModelTypes> activeModels { 
        M_MATCH ,
        M_MATCH1, 
        M_IM24,
        //M_NORMAL,
        M_LSTM    };
public:
    int mcxt[2];
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    ~PredictorIMG24(){ }
    PredictorIMG24(Settings &set);
    void update();
};
