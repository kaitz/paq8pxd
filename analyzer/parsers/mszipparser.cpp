#include "mszipparser.hpp"

MSZIPParser::MSZIPParser() {
    priority=3;
    Reset();
    inpos=0;
    name="MS Zip";
}

MSZIPParser::~MSZIPParser() {
}

// loop over input block byte by byte and report state
DetectState MSZIPParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<511) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && buf0==0x0080434b) {
            state=START;
            MSZ=i;
            MSZip=i-4,MSZipz=(buf1&0xffff);
            MSZipz=((MSZipz&0xff)<<8)+(MSZipz>>8);
            zlen=MSZipz;
            count=1;
            //test for  ((data[j]>>1)&3)!=3 or 0=uncompressed,1=fixed huf,2=dunamic huf, 3=invalid
        } else if (state==START || state==INFO) {
            const uint64_t p=i-MSZip-12ul;        
            if (p==zlen) {
                MSZip=i-4;
                zlen=(buf1&0xffff);
                zlen=((zlen&0xff)<<8) +(zlen >>8);
                if (buf0==0x0080434b) {    //32768 CK
                    MSZipz+=zlen;          //12?
                    count++;
                    state=INFO;
                } else if ((buf0&0xffff)==0x434b && zlen<32768) { //if final part <32768 CK
                    count++;
                    MSZipz+=zlen+count*8; //4 2 2
                    type=MSZIP;
                    state=END;
                    jstart=MSZ-3;
                    jend=MSZipz+jstart;
                    return state;
                } else {   
                    state=NONE;
                }
            }
        }

        inSize++;
        i++;
    }
    // Report valid block
    if (state==INFO && count>2) return INFO;
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType MSZIPParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int MSZIPParser::TypeCount() {
    return 1;
}

void MSZIPParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    MSZ=MSZip=MSZipz=zlen=count=0;
    info=i=inSize=0;
    priority=3;
}
void MSZIPParser::SetEnd(uint64_t e) {
    jend=e;
}
