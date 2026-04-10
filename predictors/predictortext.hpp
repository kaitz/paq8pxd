#pragma once
#include "predictors.hpp"
#include "../prt/mixers.hpp"
#include "../prt/EAPM.hpp"
#include "../prt/ESSE.hpp"

// Text predicor
class PredictorTXTWRT: public Predictors {
    int pr;  // next prediction
    int pr0;
    int order;
    int rlen;
    int ismatch;
    Mixers *m;
    ErrorInfo einfo;
    struct {
        APM APMs[4];
        APM1 APM1s[3];
    } Text;
    U32 count;
    U32 blenght;
    StateMap StateMaps[1];
    wrtDecoder wr;
    eSSE sse;
    int decodedTextLen,lasttag;
    int counttags,lState;
    const std::vector<MType> activeModels { 
        {M_RECORD,      SLOW|EXTREME},
        {M_MATCH,       SLOW|EXTREME},
        {M_MATCH1,      SLOW|EXTREME},
        {M_INDIRECT,    SLOW|EXTREME},
        {M_DMC,         SLOW|EXTREME},
        {M_NEST,        SLOW|EXTREME},
        {M_NORMAL,      SLOW|EXTREME},
        {M_XML,         SLOW|EXTREME},
        {M_TEXT,        SLOW|EXTREME},
        {M_WORD,        SLOW|EXTREME},
        {M_SPARSEMATCH, SLOW|EXTREME},
        {M_SPARSE,      SLOW|EXTREME},
        {M_PPM,              EXTREME},
        {M_CHART,            EXTREME},
        {M_LSTM,             EXTREME}
    };
public:
    int mcxt[9];
    PredictorTXTWRT(Settings &set);
    int p() const {/*assert(pr>=0 && pr<4096);*/ return pr;} 
    void wrt();
    void update() ;
    ~PredictorTXTWRT() {
        // printf("\n TXTWRT Count of skipped bytes %d\n",count/8);
        delete m;
    }
};
