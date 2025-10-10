#include "exeparser.hpp"

EXEParser::EXEParser() {
    priority=2;
    Reset();
    inpos=0;
    name="exe";
}

EXEParser::~EXEParser() {
}
 void EXEParser::ReadEXE() {
 if (((buf1&0xfe)==0xe8 || (buf1&0xfff0)==0x0f80) && ((buf0+1)&0xfe)==0) {
      uint8_t r=buf0>>24;             // relative address low 8 bits
      uint8_t a=((buf0>>24)+i)&0xff;  // absolute address low 8 bits
      uint64_t rdist=(i-relpos[r]);
      uint64_t adist=(i-abspos[a]);
      if (adist<rdist && adist<0x800 && abspos[a]>5) {
        e8e9last=i;
        ++e8e9count;
        if (e8e9pos==0 || e8e9pos>abspos[a]) e8e9pos=abspos[a];
      }
      else e8e9count=0;
      if (state==NONE && e8e9count>=4 && e8e9pos>5){
               jstart=e8e9pos-5;
               type=EXE;
               state=INFO;
           }
      abspos[a]=i;
      relpos[r]=i;
    }
  }  
// loop over input block byte by byte and report state
DetectState EXEParser::Parse(unsigned char *data, uint64_t len, uint64_t pos) {
    // To small? 
    if (pos==0 && len<4000) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE) {
           ReadEXE();
           if (i-e8e9last>0x4000) {
             e8e9count=0,e8e9pos=0;
            }
        } else if (state==START || state==INFO) {
            ReadEXE();
            if (i-e8e9last>0x4000) {
                jend=jstart+e8e9last;
                state=END;
                return state;
            }
        }

        inSize++;
        i++;
    }
    // type larger then block, repory info
    if (state==INFO) {
        state=START;
        return INFO;
    }
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType EXEParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int EXEParser::TypeCount() {
    return 1;
}

void EXEParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    memset(&abspos[0], 0u, sizeof(abspos));
    memset(&relpos[0], 0u, sizeof(relpos));
    e8e9count=0;
    e8e9pos=e8e9last=0;  
    info=i=inSize=0;
}
void EXEParser::SetEnd(uint64_t e) {
    jend=e;
}
