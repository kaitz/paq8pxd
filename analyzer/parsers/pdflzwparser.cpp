#include "pdflzwparser.hpp"

PDFLzwParser::PDFLzwParser() {
    priority=3;
    Reset();
    inpos=0;
    name="pdf lzw";
}

PDFLzwParser::~PDFLzwParser() {
}

// loop over input block byte by byte and report state
DetectState PDFLzwParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<256) return DISABLE;
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

        if (state==NONE && (
                    (buf4==0x2F4C5A57 && buf3==0x4465636F && buf2==0x64650D0A && buf1==0x3E3E0D0A && buf0==0x73747265) || // '/LZWDecode\r\n>>\r\nstre'
                    (buf4==0x202F4669  && buf3==0x6C746572  && buf2==0x202F4C5A  && buf1==0x57446563  && buf0==0x6F646520 ///' /Filter /LZWDecode '
                        )) ) {
            state=START;
            pLzwp=i;
        } else if (state==START) {
            if (buf0==0x616D0D0A) { // 'am\r\n'
                jstart=i;
                pLzwp=jstart-pLzwp;
                if (pLzwp>10 && pLzwp<150) { // LZWDecode and stream distance in 10-150 bytes
                    state=INFO;    
                } else {
                    state=NONE;
                }
            }
        } else if (state==INFO) {
            if (buf1==0x0D656E64 && buf0==0x73747265) { // '\rendstre'
                jend=i-8;
                type=CMP;
                state=END;
                return state;
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

dType PDFLzwParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void PDFLzwParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=buf3=buf4=0;
    pLzwp=0;
    info=i=inSize=0;
    priority=3;
}
