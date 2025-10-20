#include "jpegparser.hpp"

JPEGParser::JPEGParser() {
    priority=2;
    Reset();
    inpos=0;
    name="jpeg";
}

JPEGParser::~JPEGParser() {
}

// loop over input block byte by byte and report state
DetectState JPEGParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<256) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && /*i>3 &&*/ (((buf0&0xffffff00)==0xffd8ff00 && ((c&0xfe)==0xC0 || c==0xC4 || (c>=0xDB && c<=0xFE))) ||
                (buf0&0xfffffff0)==0xffd8ffe0)) {
            state=START;
            soi=i, app=i+2, sos=sof=0;
        } else if (state==START || state==INFO) {
            if (app==i && (buf0>>24)==0xff &&
                    ((buf0>>16)&0xff)>0xc1 && ((buf0>>16)&0xff)<0xff) app=i+(buf0&0xffff)+2;//,brute=false;
            if (app<i && (buf1&0xff)==0xff && (buf0&0xfe0000ff)==0xc0000008) sof=i;//,brute=false;
            if (sof && sof>soi && i-sof<0x1000 && (buf0&0xffff)==0xffda) {
                sos=i;
                jstart=soi-3;
                state=INFO;
            }
            if (i-soi>0x40000 && !sos) state=NONE;
            if (state==INFO){
                if (soi && (buf0&0xffff)==0xffd9) eoi=i;
                if (soi && sos && eoi && (buf0&0xffff)==0xffd8) {
                    state=END;
                    jend=jstart+i+1;
                    type=JPEG;
                    return state;
                } else if (sos && i>sos && (buf0&0xff00)==0xff00 && c!=0 && (c&0xf8)!=0xd0) {
                    state=END;
                    jend=jstart+i+1;
                    type=JPEG;
                    return state;
                }
            }
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

dType JPEGParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int JPEGParser::TypeCount() {
    return 1;
}

void JPEGParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    soi=sof=sos=app=eoi=0;
    info=i=inSize=0;
}
void JPEGParser::SetEnd(uint64_t e) {
    jend=e;
}
