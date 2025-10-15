#include "sgiparser.hpp"

sgiParser::sgiParser() {
    priority=2;
    Reset();
    inpos=0;
    name="sgi/rgb";
}

sgiParser::~sgiParser() {
}

// loop over input block byte by byte and report state
DetectState sgiParser::Parse(unsigned char *data, uint64_t len, uint64_t pos) {
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

        if (state==NONE && (buf0&0xffff)==0x01da) {
            state=START;
            rgbi=i,rgbx=rgby=0;
        } else if (state==START) {
            const int p=int(i-rgbi);
            if (p==1 && c!=0) state=NONE;
            else if (p==2 && c!=1) state=NONE;
            else if (p==4 && (buf0&0xffff)!=1 && (buf0&0xffff)!=2 && (buf0&0xffff)!=3) state=NONE;
            else if (p==6) {
                 rgbx=buf0&0xffff;
                 if (rgbx==0) state=NONE;
            } else if (p==8) {
                rgby=buf0&0xffff;
                if (rgby==0) state=NONE;
            } else if (p==10) {
                int z=buf0&0xffff;
                if (rgbx && rgby && (z==1 || z==3 || z==4)) {
                    jstart=rgbi-1+512;
                    info=rgby*z;
                    jend=jstart+rgbx*info;
                    state=END;
                    type=IMAGE8;
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

dType sgiParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int sgiParser::TypeCount() {
    return 1;
}

void sgiParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    rgbi=0;
    rgbx=rgby=0;
    info=i=inSize=0;
}
void sgiParser::SetEnd(uint64_t e) {
    jend=e;
}
