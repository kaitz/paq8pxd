#include "cdparser.hpp"

cdParser::cdParser():cdata(2353) {
    priority=1;
    Reset();
    inpos=0;
    name="cd";
}

cdParser::~cdParser() {
}

// loop over input block byte by byte and report state
DetectState cdParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<2352) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && buf1==0x00ffffff && buf0==0xffffffff) {
            state=START;
            cdi=i,cda=-1,cdm=0;cdatai=0;
            cdata[cdatai++]=0; for(int j=0;j<7;j++) cdata[cdatai++]=0xff;
        } else if (state==START || state==INFO) {
            if (cdi && i>cdi) {
                assert(cdatai<2352);
                cdata[cdatai++]=c;
                const int p=(i-cdi)%2352;
                if (p==8 && (buf1!=0xffffff00 || ((buf0&0xff)!=1 && (buf0&0xff)!=2))) {
                    if (cdscont>32) {
                        jend=i-15;
                        type=CD;
                        info=cdif;
                        pinfo=" (m"+ itos(info==1?1:2) +"/f"+ itos(info!=3?1:2) +")";
                        state=END;
                        return state;
                    }
                    cdi=0,cdatai=0,state=NONE; // FIX it ?
                }
                else if (cdatai==2352) {
                    // We have whole sector, test it
                    CDHeder *ch=(struct CDHeder*)&cdata[0];
                    int t=expand_cd_sector(&cdata[0], cda, 1); 
                    if (t!=cdm) cdm=t*(i-cdi<2352);
                    if (cdm && cda!=10 && (cdm==1 || ch->sub1==ch->sub2)) {
                        if (t && state==START) state=INFO, jstart=i-2352+1; 
                        cdif=cdm;
                        cda=bswap(ch->a)>>8;
                        if (cdm!=1 && i-cdi>2352 && ch->sub2!=cdf) cda=10;
                        if (cdm!=1) cdf=ch->sub2;
                        cdatai=0;
                        cdscont++;
                        if (cdscont>32) priority=0;// after 32 sectors set priority to 0
                    } else if (state==START) {
                        cdi=0,cdatai=0,state=NONE;
                    } else {
                        jend=i-2352+1;
                        type=CD;
                        info=cdif;
                        pinfo=" (m"+ itos(info==1?1:2) +"/f"+ itos(info!=3?1:2) +")";
                        state=END;
                        return state;
                    }
                    if (last==true && (inSize+1)==len){
                        jend=i+1;
                        type=CD;
                        info=cdif;
                        pinfo=" (m"+ itos(info==1?1:2) +"/f"+ itos(info!=3?1:2) +")";
                        state=END;
                        return state;
                    }
                } else if (cdatai>2352) {
                    // This should never happen
                    cdi=0,cdatai=0,state=NONE;
                }
            }
        }

        inSize++;
        i++;
    }
    if (state==INFO) {jend=i+1; return INFO;}
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType cdParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void cdParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    cdi=0;
    cda=cdm=cdif=0;
    cdf=0;
    cdatai=cdscont=0;
    info=i=inSize=0;
    priority=1;
}
