#include "szddparser.hpp"

SZDDParser::SZDDParser() {
    priority=3;
    LZringbuffer=nullptr;
    Reset();
    inpos=0;
    name="SZDD";
}

SZDDParser::~SZDDParser() {
    if (LZringbuffer!=nullptr) free(LZringbuffer),LZringbuffer=nullptr;
}

// loop over input block byte by byte and report state
DetectState SZDDParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
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

        if (state==NONE && ((buf0==0x88F02733 && buf1==0x535A4444 ) || (buf1==0x535A2088 && buf0==0xF02733D1)) ){
            state=START;
            fSZDD=i;
        } else if (state==START && buf0!=0 && (((i-fSZDD)==6 && (buf1&0xff00)==0x4100 && ((buf1&0xff)==0 || (buf1&0xff)>'a')&&(buf1&0xff)<'z') || (buf1!=0x88F02733   && (i-fSZDD)==4))) {
            if (buf1!=0x88F02733 && (i-fSZDD)==4) lz2=2,F+=2;  //+2 treshold
            fsizez=bswap(buf0); //uncompressed file size
            if (fsizez>0x1ffffff || fsizez<128) state=NONE;
            else {
                if (LZringbuffer!=nullptr) free(LZringbuffer),LZringbuffer=nullptr;
                LZringbuffer=(uint8_t*)calloc(N+F-1,1);
                memset(&LZringbuffer[0],' ',N-F);
                r=N-F;
                state=INFO;
                fSZDD=i+1; d.clear();rpos=0;
            }
        } else if (state==INFO) {
            if (rpos==4) {

                for(;;) {
                    // Get a byte. For each bit of this byte:
                    // 1=copy one byte literally, from input to output
                    // 0=get two more bytes describing length and position of previously-seen
                    // data, and copy that data from the ring buffer to output
                    if((flags&0x100)==0){
                        c1=d.front();d.pop_front();
                        incount++;
                        if(icount>=fsizez) break;
                        flags=c1|0xFF00;
                        rpos--;
                    }
                    if(flags & 1) {
                        c1=d.front();d.pop_front();
                        incount++;
                        if(icount>=fsizez) break;
                        icount++;
                        LZringbuffer[r]=c1;
                        r=(r+1)&(N-1);
                        rpos--;
                    } else {
                        // 0=get two more bytes describing length and position of previously-
                        // seen data, and copy that data from the ring buffer to output
                        i1=d.front();d.pop_front();
                        incount++;
                        if(icount>=fsizez) break;
                        j=d.front();d.pop_front();
                        incount++;
                        if(icount>=fsizez) break;
                        i1|=((j&0xF0)<<4);
                        j=(j&0x0F)+THRESHOLD;
                        for(k=0; k<=j; k++){
                            uint8_t a=LZringbuffer[(i1+k)&(N-1)];
                            icount++;
                            LZringbuffer[r]=a;
                            r=(r+1)&(N-1);
                        }
                        rpos--;
                        rpos--;
                    }
                    break;
                }
                if(icount>=fsizez) {
                    //printf("Dec size %d  Comp size %d\n",icount,incount);
                    jstart=fSZDD;
                    jend=jstart+incount;
                    info=icount+(lz2?(1<<25):0);
                    type=SZDD;
                    if (LZringbuffer!=nullptr) free(LZringbuffer),LZringbuffer=nullptr;
                    state=END;
                    return state;
                }
                flags>>=1;
            } 
            d.push_back(c),rpos++;
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

dType SZDDParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void SZDDParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    fSZDD=0;
    lz2=0;
    fsizez=0;
    csize=usize=0;
    rpos=0;
    N=4096,F=16,THRESHOLD=2;
    r=flags=0;
    i1=c1=j=k=0;
    icount=incount=0;
    if (LZringbuffer!=nullptr) free(LZringbuffer),LZringbuffer=nullptr;
    info=i=inSize=0;
    priority=3;
}
