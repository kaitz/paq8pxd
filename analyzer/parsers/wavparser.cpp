#include "wavparser.hpp"

WAVParser::WAVParser() {
    priority=2;
    Reset();
    inpos=0;
    name="wav";
}

WAVParser::~WAVParser() {
}

// loop over input block byte by byte and report state
DetectState WAVParser::Parse(unsigned char *data, uint64_t len, uint64_t pos) {
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

        if (state==NONE && buf0==0x52494646) {
            state=START;
            wavi=i,wavm=0;
        } else if (state==START || state==INFO) {
            int p=i-wavi;
            if (p==4) wavsize=bswap(buf0);
            else if (p==8) {
                wavtype=(buf0==0x57415645)?1:(buf0==0x7366626B)?2:0;
                if (!wavtype) state=NONE;
            }
            else if (wavtype) {
                if (wavtype==1) {
                    if (p==16 && (buf1!=0x666d7420 || bswap(buf0)!=16)) state=NONE;
                    else if (p==20) wavt=bswap(buf0)&0xffff;
                    else if (p==22) wavch=bswap(buf0)&0xffff;
                    else if (p==24) wavsr=bswap(buf0);
                    else if (p==34) wavbps=bswap(buf0)&0xffff;
                    else if (p==40+wavm && buf1!=0x64617461) wavm+=bswap(buf0)+8,wavi=(wavm>0xfffff?0:wavi);
                    else if (p==40+wavm) {
                        int wavd=bswap(buf0);
                        info2=wavsr;
                        if ((wavch==1 || wavch==2) && (wavbps==8 || wavbps==16) && wavt==1 && wavd>0 && wavsize>=wavd+36
                                && wavd%((wavbps/8)*wavch)==0 && wavsr>=0) {
                            jstart=wavi-3+44+wavm;
                            jend=jstart+wavd;
                            info=wavch+wavbps/4-3;
                            info=info+(info2<<32);
                            type=AUDIO;
                            state=END;
                            return state;
                        }
                        state=NONE;
                    }
                } else {
                    if ((p==16 && buf1!=0x4C495354) || (p==20 && buf0!=0x494E464F))
                        state=NONE;
                    else if (p>20 && buf1==0x4C495354 && (wavi*=(buf0!=0))) {
                        wavlen=bswap(buf0);
                        wavlist=i;
                    }
                    else if (wavlist){
                        p=i-wavlist;
                        if (p==8 && (buf1!=0x73647461 || buf0!=0x736D706C))
                            state=NONE;
                        else if (p==12) {
                            int wavd = bswap(buf0);
                            info2=44100;
                            if (wavd && (wavd+12)==wavlen) {
                                jstart=wavi-3+((12+wavlist-(wavi-3)+1)&~1);
                                jend=jstart+wavd;
                                info=uint64_t(1+16/4-3)+(info2<<32);
                                type=AUDIO;
                                state=END;
                                return state;
                            }
                            state=NONE;
                        }
                    }
                }
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

dType WAVParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int WAVParser::TypeCount() {
    return 1;
}

void WAVParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    wavi=wavlist=0;
    wavsize=wavch=wavbps=wavm=wavsr=wavt=wavtype=wavlen=0;
    info=info2=i=inSize=0;
}
void WAVParser::SetEnd(uint64_t e) {
    jend=e;
}
