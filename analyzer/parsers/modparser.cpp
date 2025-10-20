#include "modparser.hpp"

MODParser::MODParser() {
    priority=2;
    Reset();
    inpos=0;
    name="mod";
}

MODParser::~MODParser() {
}

// loop over input block byte by byte and report state
DetectState MODParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<128) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && ((buf0==0x4d2e4b2e || buf0==0x3643484e || buf0==0x3843484e  // M.K. 6CHN 8CHN
                        || buf0==0x464c5434 || buf0==0x464c5438) && (buf1&0xc0c0c0c0)==0 && i>=1083) ){
            state=START;
        } 
        if (state==START) {
            const int chn=((buf0>>24)==0x36?6:(((buf0>>24)==0x38 || (buf0&0xff)==0x38)?8:4));
            int len1=0; // total length of samples
            int numpat=1; // number of patterns
            for (int j=0; j<31; j++) {
                const int i1=data[inSize-1083+42+j*30];
                const int i2=data[inSize-1083+42+j*30+1];
                len1+=i1*512+i2*2;
            }
            for (int j=0; j<128; j++) {
                const int x=data[inSize-131+j];
                if (x+1>numpat) numpat=x+1;
            }
            if (numpat<65) {
                jstart=i+numpat*256*chn+1;
                jend=jstart+len1;
                type=AUDIO;
                info=4;
                pinfo=" ("+audiotypes[(info&31)%4+(info>>7)*2]+")";
                state=END;
                return state;
            }
            state=NONE;
        }

        inSize++;
        i++;
    }
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType MODParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int MODParser::TypeCount() {
    return 1;
}

void MODParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    
    info=i=inSize=0;
}
void MODParser::SetEnd(uint64_t e) {
    jend=e;
}
