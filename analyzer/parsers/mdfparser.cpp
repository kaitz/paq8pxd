#include "mdfparser.hpp"

mdfParser::mdfParser():cdata(2353) {
    priority=1;
    Reset();
    inpos=0;
    name="mdf";
}

mdfParser::~mdfParser() {
}

// loop over input block byte by byte and report state
DetectState mdfParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<(2352+96)) return DISABLE;
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
            cdi=i,cda=-1,cdm=0;cdatai=mdfdatai=0;
            cdata[cdatai++]=0; for(int j=0;j<7;j++) cdata[cdatai++]=0xff;
        } else if (state==START || state==INFO) {
            if (cdi && i>cdi) {
                assert(cdatai<2352);
                cdata[cdatai++]=c;
                const int p=(i-cdi)%2352;
                if (cdatai==2352) {
                    // We have whole sector, test it
                    if (mdfa==0 && state==INFO) {
                        uint32_t mdf= (cdata[96]<<24)+(cdata[96+1]<<16)+(cdata[96+2]<<8)+cdata[96+3];
                        uint32_t mdf1=(cdata[96+4]<<24)+(cdata[96+5]<<16)+(cdata[96+6]<<8)+cdata[96+7];
                        if (mdf==0x00ffffff && mdf1==0xffffffff ) mdfa=cdi,cdi=cdm=0; //drop to mdf mode?
                    }
                    if (cdscont>1) {
                        // More then on valid cd sector
                        state=DISABLE;
                        return state;
                    }
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
                    } else if (state==START && mdfa==0) {
                        cdi=0,cdatai=0,state=NONE;
                    }
                    cdatai=0;
                } else if (cdatai>2352) {
                    // This should never happen
                    cdi=0,cdatai=0,state=NONE;
                }
            } else if (i>mdfa) {
                
                const int p=(i-mdfa)%2448;
                if (p==8 && (buf1!=0xffffff00 || ((buf0&0xff)!=1 && (buf0&0xff)!=2))) {
                    mdfa=0;
                    jend=i-p-7;
                    type=MDF;
                    info=0;
                    state=END;
                    if (mdfdatai>4)
                        return state;
                    state=NONE;
                    jend=jstart=0;type=DEFAULT;
                } else mdfdatai++;
                if (mdfdatai>32) priority=0;// after 32 sectors set priority to 0
            }
            if (last==true && (inSize+1)==len){
                mdfa=0;
                const int p=(i+1-mdfa)%2448;
                jend=i-p+1;
                type=MDF;
                info=0;
                state=END;
                if (mdfdatai>4)
                        return state;
                state=NONE;
                jend=jstart=0;type=DEFAULT;
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

dType mdfParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void mdfParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    mdfa=0;
    cdi=0;
    cda=cdm=cdif=0;
    cdf=0;
    cdatai=mdfdatai=0;
    cdscont=0;
    info=i=inSize=0;
    priority=1;
}
