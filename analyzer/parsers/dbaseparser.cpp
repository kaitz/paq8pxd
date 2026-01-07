#include "dbaseparser.hpp"

dBaseParser::dBaseParser() {
    priority=3;
    Reset();
    inpos=0;
    name="dBase";
}

dBaseParser::~dBaseParser() {
}

// loop over input block byte by byte and report state
DetectState dBaseParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
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

        if (state==NONE && ((c&7)==3 || (c&7)==4 || (c>>4)==3|| c==0xf5 || c==0x30) ) {
            state=START;
            dbasei=i+1;
            dbase.Version=((c>>4)==3)?3:c&7;
            dbase.HeaderLength=dbase.Start=dbase.nRecords=dbase.RecordLength=0;
        } else if (state==START) {
            const int p=int(i-dbasei+1);
            if (p==2 && !(c>0 && c<13)) state=NONE;      //month
            else if (p==3 && !(c>0 && c<32)) state=NONE; //day
            else if (p==7 && !((dbase.nRecords=bswap(buf0)) > 0 && dbase.nRecords<0xFFFFF)) state=NONE;
            else if (p==9 && !((dbase.HeaderLength=((buf0>>8)&0xff)|(c<<8)) > 32 &&
             ( ((dbase.HeaderLength-32-1)%32)==0 || (dbase.HeaderLength>255+8 && (((dbase.HeaderLength-=255+8)-32-1)%32)==0) )) ) state=NONE;
            else if (p==11 && !(((dbase.RecordLength=((buf0>>8)&0xff)|(c<<8))) > 8) ) state=NONE;
            else if (p==15 && ((buf0&0xfffffefe)!=0 && ((buf0>>8)&0xfe)>1 && (buf0&0xfe)>1)) state=NONE;
            else if (p==16 && dbase.RecordLength>4000) state=NONE;
            else if (p==17) term=i+dbase.HeaderLength-18;
            else if (p==term) {
                //Field Descriptor terminator
                if (c!=0xd) state=NONE;
                else {
                    jstart=dbasei-1+dbase.HeaderLength;
                    jend=jstart+dbase.nRecords*dbase.RecordLength+1;
                    info=dbase.RecordLength;
                    type=DBASE;
                    state=INFO;
                }
            }
            else if (p>dbase.HeaderLength && p>68) state=NONE; // Fail if past Field Descriptor terminator
        } else if (state==INFO && i==(jend-1)) {
            // Is file end marker? Fail if not present.
            if (c==0x1a) {
                state=END;
                return state;
            } else {
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

dType dBaseParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int dBaseParser::TypeCount() {
    return 1;
}

void dBaseParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    memset(&dbase, 0, sizeof(dBASE));
    dbasei=term=0;
    info=i=inSize=0;
    priority=3;
}
void dBaseParser::SetEnd(uint64_t e) {
    jend=e;
}
