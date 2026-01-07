#include "ascii85parser.hpp"

ascii85Parser::ascii85Parser() {
    priority=3;
    Reset();
    inpos=0;
    name="base85";
}

ascii85Parser::~ascii85Parser() {
}

bool ascii85Parser::is_base85(unsigned char c) {
    return (isalnum(c) || (c==13) || (c==10) || (c=='y') || (c=='z') || (c>='!' && c<='u'));
}
// loop over input block byte by byte and report state
DetectState ascii85Parser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
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

        if (state==NONE && (
                    (buf0==0x65616D0A && (buf1&0xffffff)==0x737472) || //'stream\n' 
                    (buf0==0x616D0D0A && buf1==0x73747265) || // 'stream\r\n' 
                    (buf0==0x6167650A && buf1==0x6F4E696D) || // 'oNimage\n' 
                    (buf0==0x6167650A && buf1==0x7574696D) || // 'utimage\n' 
                    (buf0==0x6167650A && (buf1&0xffffff)==0x0A696D) //'\nimage\n'
                    )) {
            state=START;
            b85p=i-6,b85h=0,b85slen=0;//,b85lcount=0; // 
            b85h=i+1,b85slen=b85h-b85p;
            base85start=i;//+1;
            if (b85slen>128) state=NONE; //drop if header is larger 
        } else if (state==START || state==INFO) {
            if  ((buf0&0xff)==0x0d && b85line==0) {
                b85line=i-base85start;//,b85nl=i+2;//capture line lenght
                if (b85line<=25 || b85line>255) state=NONE;
            } else if ((buf0&0xff)==0x7E)  { //if padding '~' or '=='
                base85end=i-1;//2
                state=NONE;
                if (((base85end-base85start)>60) && ((base85end-base85start)<base85max)) {
                    jstart=b85h+b85slen-7;
                    jend=jstart+base85end -base85start;
                    type=BASE85;
                    state=END;
                    return state;
                }
            } else if ( (is_base85(c))) {
                state=INFO;
            } else if  ((buf0&0xff)==0x0d && b85line!=0) {
                if (b85line!=i-base85start) state=NONE;
            } else  {
                state=NONE;
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

dType ascii85Parser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int ascii85Parser::TypeCount() {
    return 1;
}

void ascii85Parser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    b85s=b85s1=b85p=b85slen=b85h=0;
    base85start=base85end=b85line=0;
    info=i=inSize=0;
    priority=3;
}
void ascii85Parser::SetEnd(uint64_t e) {
    jend=e;
}
