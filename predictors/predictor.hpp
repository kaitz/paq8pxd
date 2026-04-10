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
    const std::vector<MType> activeModels= { 
        {M_RECORD,      SLOW|EXTREME},
        {M_MATCH,       SLOW|EXTREME},
        {M_MATCH1,      SLOW|EXTREME},
        {M_DISTANCE,    SLOW|EXTREME},
        {M_INDIRECT,    SLOW|EXTREME},
        {M_DMC,         SLOW|EXTREME},
        {M_NEST,        SLOW|EXTREME},
        {M_NORMAL,      SLOW|EXTREME},
        {M_XML,         SLOW|EXTREME},
        {M_TEXT,        SLOW|EXTREME},
        {M_WORD,        SLOW|EXTREME},
        {M_LINEAR,      SLOW|EXTREME},
        {M_SPARSEMATCH, SLOW|EXTREME},
        {M_SPARSE_Y,    SLOW|EXTREME},
        {M_PPM,              EXTREME},
        {M_CHART,            EXTREME},
        {M_LSTM,             EXTREME}
    };
    bool palactive;
    int pal[256];
    int paloff,TCOLORS;
public:
    int mcxt[9];
    Predictor(Settings &);
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    void update();
    ~Predictor() {
        delete m;
    }
};
