#include "aiffparser.hpp"

AIFFParser::AIFFParser() {
    priority=2;
    Reset();
    inpos=0;
    name="AIFF";
}

AIFFParser::~AIFFParser() {
}

// loop over input block byte by byte and report state
DetectState AIFFParser::Parse(unsigned char *data, uint64_t len, uint64_t pos) {
    // To small? 
    if (pos==0 && len<128) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        int c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && buf0==0x464f524d) { // 'FORM'
            state=START;
            aiff=i,aiffs=0;
        } else if (state==START) {
            const int p=int(i-aiff);
            if (p==12 && (buf1!=0x41494646 || buf0!=0x434f4d4d)) state=NONE; // 'AIFF' 'COMM'
            else if (p==24) {
                const int bits=buf0&0xffff, chn=buf1>>16;
                if ((bits==8 || bits==16) && (chn==1 || chn==2)) aiffm=chn+bits/4+1; else state=NONE;
            } else if (p==42+aiffs && buf1!=0x53534e44) {
                aiffs+=(buf0+8)+(buf0&1);
                if (aiffs>0x400) state=NONE;
            }
            else if (p==42+aiffs) {
                jstart=aiff-3+54+aiffs;
                jend=jstart+buf0-8;
                info=aiffm;
                state=END;
                type=AUDIO;
                return state;
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

dType AIFFParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int AIFFParser::TypeCount() {
    return 1;
}

void AIFFParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    aiff=0;
    aiffm=aiffs=0; 
    info=i=inSize=0;
}
void AIFFParser::SetEnd(uint64_t e) {
    jend=e;
}
