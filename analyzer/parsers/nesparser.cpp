#include "nesparser.hpp"

NesParser::NesParser() {
    priority=3;
    Reset();
    inpos=0;
    name="nes";
}

NesParser::~NesParser() {
}

// loop over input block byte by byte and report state
DetectState NesParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<(0x3FFF+16)) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && buf0==0x4E45531A) {
            state=START;
            nesh=i,nesp=0;
        } else if (state==START) {
            const int p=int(i-nesh);
            if (p==1) nesp=buf0&0xff; //count of pages*0x3FFF
            else if (p==2) nesc=buf0&0xff; //count of CHR*0x1FFF
            else if (p==6 && ((buf0&0xfe)!=0) )nesh=0; // flags 9
            else if (p==11 && (buf0!=0) )nesh=0;
            else if (p==12) {
                if (nesp>0 && nesp<129) {
                    jstart=nesh-3;
                    jend=jstart+nesp*0x3FFF+nesc*0x1FFF+15;
                    type=NESROM;
                    state=END;
                    return state;
                }
                state=NONE;
            }
        }

        inSize++;
        i++;
    }
    // Are we still reading data for our type
    if (state!=NONE)
        return DATA;
    else return NONE;
}

dType NesParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void NesParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=0;
    nesh=nesp=nesc=0;
    info=i=inSize=0;
    priority=3;
}

