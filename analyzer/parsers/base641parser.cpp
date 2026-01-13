#include "base641parser.hpp"

base64_1Parser::base64_1Parser() {
    priority=3;
    Reset();
    inpos=0;
    name="base64 1";
}

base64_1Parser::~base64_1Parser() {
}

// loop over input block byte by byte and report state
DetectState base64_1Parser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
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

        if (state==NONE && (
                    (buf1==0x3b626173 && buf0==0x6536342c) ||  // ';base64,'
                    (buf1==0x215b4344 && buf0==0x4154415b)     // '![CDATA['
                    )) {
            state=START;
            b64h=i+1,base64start=i+1;
        } else if (state==START || state==INFO) {
            if (isalnum(c) || (c=='+') || (c=='/') || (c=='=')) {
                state=INFO;
            } else {
                base64end=i;
                state=NONE;
                if ((base64end-base64start)>128) {
                    jstart=b64h;
                    jend=jstart+base64end-base64start;
                    type=BASE64;
                    state=END;
                    return state;
                }
            }
        }

        inSize++;
        i++;
    }
    if (state==INFO && (i-jstart)>64000) { priority=0;}
    if (state==INFO) return INFO;
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType base64_1Parser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void base64_1Parser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    base64end=b64h=base64start=0;
    info=i=inSize=0;
    priority=3;
}
