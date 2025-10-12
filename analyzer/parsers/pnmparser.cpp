#include "pnmparser.hpp"

PNMParser::PNMParser() {
    priority=2;
    Reset();
    inpos=0;
    name="pnm";
}

PNMParser::~PNMParser() {
}

// loop over input block byte by byte and report state
DetectState PNMParser::Parse(unsigned char *data, uint64_t len, uint64_t pos) {
    // To small? 
    if (pos==0 && len<128) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        int c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (buf0&0xfff0ff)==0x50300a) {
            pgmn=(buf0&0xf00)>>8;
            if ((pgmn>=4 && pgmn<=6) || pgmn==7) {
                pgm=i,pgm_ptr=pgmw=pgmh=pgmc=pgmcomment=pamatr=pamd=0;   
                state=START;
            }
        } else if (state==START) {
            if (i-pgm==1 && c==0x23) pgmcomment=1; //pgm comment
            if (!pgmcomment && pgm_ptr) {
                int s=0;
                if (pgmn==7) {
                    if ((buf1&0xffff)==0x5749 && buf0==0x44544820) pgm_ptr=0, pamatr=1; // WIDTH
                    if ((buf1&0xffffff)==0x484549 && buf0==0x47485420) pgm_ptr=0, pamatr=2; // HEIGHT
                    if ((buf1&0xffffff)==0x4d4158 && buf0==0x56414c20) pgm_ptr=0, pamatr=3; // MAXVAL
                    if ((buf1&0xffff)==0x4445 && buf0==0x50544820) pgm_ptr=0, pamatr=4; // DEPTH
                    if ((buf2&0xff)==0x54 && buf1==0x55504c54 && buf0==0x59504520) pgm_ptr=0, pamatr=5; // TUPLTYPE
                    if ((buf1&0xffffff)==0x454e44 && buf0==0x4844520a) pgm_ptr=0, pamatr=6; // ENDHDR
                    if (c==0x0a) {
                        if (pamatr==0) pgm=0;
                        else if (pamatr<5) s=pamatr;
                        if (pamatr!=6) pamatr=0;
                    }
                }
                else if ((c==0x20|| c==0x0a) && !pgmw) s=1;
                else if (c==0x0a && !pgmh) s=2;
                else if (c==0x0a && !pgmc && pgmn!=4) s=3;
                if (s) {
                    if (pgm_ptr>=32) pgm_ptr=31;
                    pgm_buf[pgm_ptr++]=0;
                    int v=atoi(&pgm_buf[0]);
                    if (v<0 || v>20000) v=0;
                    if (s==1) pgmw=v; else if (s==2) pgmh=v; else if (s==3) pgmc=v; else if (s==4) pamd=v;
                    if (v==0 || (s==3 && v>255)) pgm=0; else pgm_ptr=0;
                }
            }
            if (!pgmcomment) pgm_buf[pgm_ptr++]=((c>='0' && c<='9') || ' ')?c:0;
            if (pgm_ptr>=32) pgm=pgm_ptr=0;
            if (i-pgm>255) pgm=pgm_ptr=0;
            if (pgmcomment && c==0x0a) pgmcomment=0;
            if (pgmw && pgmh && !pgmc && pgmn==4) {
                jstart=pgm-2+i-pgm+3;
                jend=jstart+((pgmw+7)/8)*pgmh;
                type=IMAGE1;
                info=(pgmw+7)/8;
                state=INFO;
            } else if (pgmw && pgmh && pgmc && (pgmn==5 || (pgmn==7 && pamd==1 && pamatr==6))) {
                jstart=pgm-2+i-pgm+3;
                jend=jstart+pgmw*pgmh;
                type=IMAGE8GRAY;
                info=pgmw;
                state=INFO;
            } else if (pgmw && pgmh && pgmc && (pgmn==6 || (pgmn==7 && pamd==3 && pamatr==6))) {
                jstart=pgm-2+i-pgm+3;
                jend=jstart+pgmw*3*pgmh;
                info=pgmw*3;
                type=IMAGE24;
                state=INFO;
            } else if (pgmw && pgmh && pgmc && (pgmn==7 && pamd==4 && pamatr==6)) {
                jstart=pgm-2+i-pgm+3;
                jend=jstart+pgmw*4*pgmh;
                type=IMAGE32;
                info=pgmw*4;
                state=INFO;
            }
        } else if (state==INFO && i==(jend-1)) {
            state=END;
            return state;
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

dType PNMParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int PNMParser::TypeCount() {
    return 1;
}

void PNMParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=0;
    pgm=0;
    pgmcomment=pgmw=pgmh=pgm_ptr=pgmc=pgmn=pamatr=pamd=0;
    info=i=inSize=0;
}
void PNMParser::SetEnd(uint64_t e) {
    jend=e;
}
