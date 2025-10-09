#include "textparser.hpp"

TextParser::TextParser() {
    priority=4;
    Reset();
    inpos=0;
    name="text";
}

TextParser::~TextParser() {
}

// loop over input block byte by byte and report state
DetectState TextParser::Parse(unsigned char *data, uint64_t len, uint64_t pos) {
    
    // To small? 
    if (pos==0 && len<128) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
        binLen=0; // reset after every block
    }
    
    while (inSize<len) {
        int c=data[inSize];
        if (state==NONE && ((c<128 && c>=32) || c==10 || c==13 || c==0x12 || c==9)) {
            txtStart=i;
            state=START;
        } else if (state==START || state==INFO) {
            if ((c<128 && c>=32) || c==10 || c==13 || c==0x12 || c==9) {
                ++txtLen;
                if (c==32) spaces++;
                // When valid text and at least one space
                if (txtLen>txtMinLen && state==START && spaces!=0){
                    type=TEXT; 
                    state=INFO;
                    jstart=txtStart;
                    return state;
                }
            } else {
                // Text ended?
                if (binLen<25) {
                    binLen++;
                } else {
                    jend=i;
                    if (txtLen>txtMinLen) state=END;
                }
            }
        } else if (state==END && i>=(jend)) {
           if (txtLen<txtMinLen) state=NONE;
            jend=i;
            return state;
        }

        inSize++;
        i++;
    }
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType TextParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int TextParser::TypeCount() {
    return 1;
}

void TextParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=0;
    info=i=inSize=txtStart=binLen=spaces=txtLen=0;
    txtMinLen=65536;
}

void TextParser::SetEnd(uint64_t e) {
     jend=e;
}
