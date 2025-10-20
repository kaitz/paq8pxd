#include "decaparser.hpp"

DECaParser::DECaParser() {
    priority=2;
    Reset();
    inpos=0;
    name="DECa";
}

DECaParser::~DECaParser() {
}

// loop over input block byte by byte and report state
DetectState DECaParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<4096) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        int c=data[inSize];
        buf0=(buf0<<8)+c;
        
        dec.idx=i&3u;
        dec.opcode=bswap(buf0);
        dec.count[dec.idx]=((i >= 3u) && DECAlpha::IsValidInstruction(dec.opcode)) ? dec.count[dec.idx]+1u : dec.count[dec.idx]>>3u;
        dec.opcode >>= 21u;
        if (state==NONE && (dec.branches[dec.idx]>=16u) ){
            state=START;
        } else if (state==START || state==INFO || (dec.opcode==(0x34u<<5u) + 26u) && (dec.count[dec.idx]>4u)) {
            //test if bsr opcode and if last 4 opcodes are valid
            if ( (dec.opcode==(0x34u<<5u)+ 26u) && (dec.count[dec.idx]>4u) ) {
                uint32_t const absAddrLSB=dec.opcode & 0xFFu; // absolute address low 8 bits
                uint32_t const relAddrLSB=((dec.opcode & 0x1FFFFFu) + static_cast<std::uint32_t>(i)/4u) & 0xFFu; // relative address low 8 bits
                uint64_t const absPos=dec.absPos[absAddrLSB];
                uint64_t const relPos=dec.relPos[relAddrLSB];
                uint64_t const curPos=i;
                if ((absPos>relPos) && (curPos<absPos+0x8000ull) && (absPos>16u) && (curPos>absPos+16ull) && (((curPos-absPos) & 3ull) == 0u)) {
                    dec.last = curPos;
                    dec.branches[dec.idx]++;      
                    if ((dec.offset==0u) || (dec.offset>dec.absPos[absAddrLSB])) {
                        std::uint64_t const addr=curPos-(dec.count[dec.idx]-1u)*4ull;          
                        dec.offset=((i>0u) /*&& (start == prv_start)*/) ? dec.absPos[absAddrLSB] : std::min<std::uint64_t>(dec.absPos[absAddrLSB], addr);
                    }
                }
                else
                    dec.branches[dec.idx]=0u;
                dec.absPos[absAddrLSB]=dec.relPos[relAddrLSB]=curPos;
            }
            
            if (state==START) {
                info=0;
                type=DECA;
                state=INFO;
                jstart=dec.offset-dec.offset%4;
                jend=0;
                return state;
            }

            if ((i>dec.last+(type==DECA?0x8000ull:0x4000ull)) && (dec.count[dec.offset&3]==0u)) {
                if (type==DECA) {
                    state=END;
                    jend=dec.last-dec.last%4;
                    return state;
                }
                if (state==START) {
                    dec.last=0u, dec.offset=0u;
                    memset(&dec.branches[0], 0u, sizeof(dec.branches));
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

dType DECaParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int DECaParser::TypeCount() {
    return 1;
}

void DECaParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    bmp=bpp=x=y=of=size=hdrless=info=inSize=i=0;
    dec.last = 0u, dec.offset = 0u;
    memset(&dec.branches[0], 0u, sizeof(dec.branches));
}
void DECaParser::SetEnd(uint64_t e) {
    jend=e;
}
