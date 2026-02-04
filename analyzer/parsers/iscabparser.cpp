#include "iscabparser.hpp"

ISCABParser::ISCABParser():iscF(0) {
    priority=2;
    Reset();
    inpos=0;
    name="IS-CAB";
}

ISCABParser::~ISCABParser() {
}

// loop over input block byte by byte and report state
DetectState ISCABParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && rec==false && len<(512+2+4)) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
        if (rec && state==END && iscFiles==0) state=NONE,priority=2,rec=false; // revusive mode ended
    }
    
    while (inSize<len && rec==false) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && buf0==0x49536328) { // 'ISc('
            state=START;
            isc=relAdd=i-3;
            jstart=relAdd;
            iscFiles=scount=spos=0;
        } else if (state==START && (i-isc)==511 && inSize>510) {
            CABHeader &chdr=(CABHeader&)data[inSize-511];
            if (chdr.ofsCompData==512 && isc==0 && (chdr.Version&0xff000000)==0x01000000 && ((chdr.Version >> 12) & 0xf)==5) state=INFO;
            else isc=jstart=scount=0,state=NONE;
        } else if (state==INFO) {
            if (spos) spos--;
            else scount++;
            if (scount==2 && spos==0) {
                spos=c*256+((buf0>>8)&0xff);
                if (spos<3) scount=spos=0,state=NONE;
                ISCfile tf;
                tf.start=(iscFiles==0?isc+512+2:i+1);
                tf.size=spos;
                tf.p=P_DEF;
                iscF.push_back(tf);
                iscFiles++;
                scount=0;
                if ((pos+len)==(i+spos+1)) {
                    scount=spos=0,state=NONE;
                    jstart=0;
                    jend=0;
                    state=END;
                    pinfo=" IS CAB fragments "+itos(iscFiles);
                    rec=true;
                } else if (last==true && (i+spos+1) >(pos+len)) { //fail
                    isc=jstart=scount=0,state=NONE;
                    iscF.clear();
                }
            }
        }

        inSize++;
        i++;
    }
    if (rec && state==END && iscFiles) { 
        // recursive mode, report all iso file ranges
        // file extension based type parser set in info
        ISCfile iscfile=iscF[iscF.size()-iscFiles];
        jstart=iscfile.start-(relAdd-isc);
        jend=jstart+iscfile.size;
        //printf("File %d %d %d\n",iscF.size()-iscFiles,jstart,jend);
        relAdd+=jend-isc;
        relAdd-=isc;
        type=ZLIB;
        info=0;
        isc=0;
        if ((iscF.size()-iscFiles)==1) pinfo="";
        iscFiles--;
        if (iscFiles==0) iscF.clear(),rec=false;
        return state;        
    }
    if (state==INFO) {jend=i+1; return INFO;}
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType ISCABParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=rec;
    return t;
}

void ISCABParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    isc=0,scount=0;
    relAdd=0;
    priority=2;
    iscFiles=0; 
    iscF.clear();
    info=i=inSize=0;
    rec=false;
}
