#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"

// Jpeg predicor
class PredictorJPEG: public Predictors {
    int pr;
    Mixers *m;
    ErrorInfo einfo;
    struct {
        APM APMs[1];
    } Jpeg;
    bool Bypass; 
    const std::vector<MType> activeModels { 
        {M_JPEG,   SLOW|EXTREME},
        {M_MATCH,  SLOW|EXTREME},
        {M_MATCH1, SLOW|EXTREME},
        {M_NORMAL, SLOW|EXTREME}
    };
public:
    int mcxt[7];
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    ~PredictorJPEG() {   //printf("\n JPEG Count of skipped bytes %d\n",x.count);
        delete m;
    }
    PredictorJPEG(Settings &set);
    void update() ;
};
