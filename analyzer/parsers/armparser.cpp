#include "armparser.hpp"

ARMParser::ARMParser() {
    priority=3;
    Reset();
    inpos=0;
    name="arm";
}

ARMParser::~ARMParser() {
}

uint32_t ARMParser::OpC(uint32_t opcode) {
    uint32_t ar=opcode>>30;       // 1 = 64 bit
    if (ar==0) return 0;
    uint32_t op=(opcode>>25)&15;
    if (ar==0 && op==0) {         // Reserved
        return -1;
    } else if (ar==1 && op==0) {  // Scalable matrix extension
        return 1;
    } else if (op==2) {           // Scalable vector extension 
        return 1;
    } else if ((op>>1)==4) {      // Immediate
        return 1;
    } else if ((op>>1)==5) {      // Branch, exception, system register
        return 1;
    } else if ((op&0xf5)==4) {    // Load, store
        return 1;
    } else if ((op&0xf7)==5) {    // Register
        return 1;
    } else if ((op&0xf7)==7) {    // Floating
        return 1;
    } else {                      // Invalid
        return 0;
    }
}
void ARMParser::ReadARM() {
    uint32_t op=(buf0)>>26; 
    // Test if bl opcode and if last 4 opcodes are valid 
    if ((i%3) && op==0x25  && OpC(buf0) && OpC(buf1)==1 && OpC(buf2)==1 && OpC(buf3)==1 && OpC(buf4)==1) {
        int a=(buf0)&0xff;
        int r=(buf0)&0x3FFFFFF;
        r+=(i)/4;  
        r=r&0xff;
        int rdist=int(i-relposARM[r]);
        int adist=int(i-absposARM[a]);
        if (adist<rdist && adist<0x3FFFFF && absposARM[a]>16 &&  adist>16 && adist%4==0) {
            ARMlast=i;
            ++ARMcount;
            if (ARMpos==0 || ARMpos>absposARM[a]) ARMpos=absposARM[a];
        }
        else ARMcount=0;
        if (state==NONE && ARMcount>=18 && ARMpos>16){
            jstart=ARMpos-ARMpos%4;
            type=ARM;
            state=INFO;
        }
        absposARM[a]=i;
        relposARM[r]=i;
    }
}
// loop over input block byte by byte and report state
DetectState ARMParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<4000) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf4=(buf4<<8)|(buf3>>24);
        buf3=(buf3<<8)|(buf2>>24);
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE) {
            ReadARM();
            if (i-ARMlast>0x4000) {
                ARMcount=0,ARMpos=0,ARMlast=0;
            }
        } else if (state==INFO) {
            ReadARM();
            if (i-ARMlast>0x4000 || i==(pos+len-1) && last) {
                jend=ARMlast;
                state=END;
                return state;
            }
        }

        inSize++;
        i++;
    }
    // type larger then block, repory info
    if (state==INFO) {
        return INFO;
    }
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType ARMParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void ARMParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=0;
    buf0=buf1=buf2=buf3=buf4=0;
    memset(&absposARM[0], 0u, sizeof(absposARM));
    memset(&relposARM[0], 0u, sizeof(relposARM));
    ARMcount=0;
    ARMpos=ARMlast=0;  
    info=i=inSize=0;
    priority=3;
}
