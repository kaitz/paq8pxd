#include "base642parser.hpp"

base64_2Parser::base64_2Parser() {
    priority=2;
    Reset();
    inpos=0;
    name="base64 2";
}

base64_2Parser::~base64_2Parser() {
}

bool base64_2Parser::is_base64(unsigned char c) {
    return (isalnum(c) || (c=='+') || (c=='/')|| (c==10) || (c==13));
}

// loop over input block byte by byte and report state
DetectState base64_2Parser::Parse(unsigned char *data, uint64_t len, uint64_t pos) {
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
                buf0==0x73653634 && ((buf1&0xffffff)==0x206261 || (buf1&0xffffff)==0x204261) // ' base64' ' Base64'
                )) {
            state=START;
            b64s=1,b64p=i-6,b64h=0,b64slen=0,b64lcount=0;
        } else if (state==START || state==INFO) {
            if (b64s==1 && buf0==0x0D0A0D0A) {
                b64s=2,b64h=i+1,b64slen=b64h-b64p;
                base64start=i+1;
                if (b64slen>192) b64s=0,state=NONE; // drop if header is larger 
            } else if (b64s==2 && (buf0&0xffff)==0x0D0A && b64line==0) {
                b64line=i-base64start,b64nl=i+2; // capture line lenght
                if (b64line<=4 || b64line>255) b64s=0,state=NONE;
            } else if (b64s==2 && (buf0&0xffff)==0x0D0A  && b64line!=0 && (buf0&0xffffff)!=0x3D0D0A && buf0!=0x3D3D0D0A) {
                if (i-b64nl+1<b64line && buf0!=0x0D0A0D0A) { // if smaller and not padding
                    base64end=i-1;
                    if (((base64end-base64start)>512) && ((base64end-base64start)<base64max)){
                        state=NONE;
                        jstart=b64h;
                        jend=jstart+base64end-base64start;
                        type=BASE64;
                        state=END;
                        return state;
                    }
                } else if (buf0==0x0D0A0D0A) { // if smaller and not padding
                    base64end=i-1-2;
                    if (((base64end-base64start)>512) && ((base64end-base64start)<base64max)) {
                        jstart=b64h;
                        jend=jstart+base64end-base64start;
                        type=BASE64;
                        state=END;
                        return state;
                    }
                    state=NONE;
                }
                b64nl=i+2; //update 0x0D0A pos
                b64lcount++;
            } else if (b64s==2 && ((buf0&0xffffff)==0x3D0D0A || buf0==0x3D3D0D0A)) { //if padding '=' or '=='
                base64end=i-1;
                state=NONE;
                if (((base64end-base64start)>512) && ((base64end-base64start)<base64max)){
                    jstart=b64h;
                    jend=jstart+base64end-base64start;
                    type=BASE64;
                    state=END;
                    return state;
                }
            } else if (b64s==2 && (is_base64(c) || c=='=')) state=INFO;
            else if (b64s==2) state=NONE;
        }

        inSize++;
        i++;
    }
    if (state==INFO) return INFO;
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType base64_2Parser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int base64_2Parser::TypeCount() {
    return 1;
}

void base64_2Parser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    b64s=b64p=b64slen=b64h=0;
    base64start=base64end=b64line=b64nl=b64lcount=0;
    info=i=inSize=0;
}
void base64_2Parser::SetEnd(uint64_t e) {
    jend=e;
}
