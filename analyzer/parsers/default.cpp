#include "default.hpp"
 
DefaultParser::DefaultParser() {
    priority=MAX_PRI-1;
    Reset();
    inpos=0;
    name="default";
}

DefaultParser::~DefaultParser() {
}

// loop over input block byte by byte and report state
DetectState DefaultParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    if (state==NONE) {
        state=START;
        jstart=pos;
    } else if (state==START) {
        state=INFO;
        return state;
    }

    jend=jend+len;
    return DATA;
}

dType DefaultParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int DefaultParser::TypeCount() {
    return 1;
}

void DefaultParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=0;
    info=i=0;
    priority=MAX_PRI-1;
}

void DefaultParser::SetEnd(uint64_t e) {
     jend=e;
}
