#include "uueparser.hpp"

uueParser::uueParser() {
    priority=2;
    Reset();
    inpos=0;
    name="uue";
}

uueParser::~uueParser() {
}

// loop over input block byte by byte and report state
DetectState uueParser::Parse(unsigned char *data, uint64_t len, uint64_t pos) {
    // To small? 
    if (pos==0 && len<128) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (                
                    (buf0==0x67696E20 && (buf1&0xffffff)==0x0A6265) || //'\n begin '
                    (buf2==0x0A424547&& buf1==0x494E2D2D&& buf0==0x63757420) || // '\nBEGIN--cut ' //sample?
                    ((buf0&0xffffff00)==0x696E2000 && (buf1&0xffffff)==0x626567 && (buf0&0xff)>='0' && (buf0&0xff)<='9' ) // 'begin x' where x=0...9
                    )) {
            state=START;
            uuds=1,uudp=i-8,uudh=0,uudslen=0,uudlcount=0;
            if ((buf0&0xff)>='0' && (buf0&0xff)<='9') uudp=i-6;
        } else if (state==START || state==INFO) {
            if (uuds==1 && (buf0&0xffff)==0x0A4D ) {
                uuds=2,uudh=i,uudslen=uudh-uudp;
                uudstart=i;
                if (uudslen>40) uuds=0,state=NONE;  //drop if header is larger 
            }
            else if (uuds==1 && (buf0&0xffff)==0x0A62 ) uuds=0,state=NONE ; //reset for begin
            else if (uuds==2 && (buf0&0xff)==0x0A && uudline==0) {
                uudline=i-uudstart,uudnl=i;      //capture line lenght
                if (uudline!=61) uuds=uudline=0; //drop if not
            }
            else if (uuds==2 &&( (buf0&0xff)==0x0A || (buf0==0x454E442D && (buf1&0xff)==0x0A))  && uudline!=0){// lf or END-
                if ( (i-uudnl+1<uudline && (buf0&0xffff)!=0x0A0A) ||  ((buf0&0xffff)==0x0A0A) ) { // if smaller and not padding
                    uudend=i-1;
                    if ( (((uudend-uudstart)>128) && ((uudend-uudstart)<512*1024))  ){
                        uuds=0;
                        jstart=uudh;
                        jend=jstart+uudend -uudstart;
                        type=UUENC;
                        info=uuc;
                        state=END;
                        return state;
                    }
                }
                else if(buf0==0x454E442D) { // 'END-'
                    uudend=i-5;
                    jstart=uudh;
                    jend=jstart+uudend -uudstart;
                    type=UUENC;
                    info=uuc;
                    state=END;
                    return state;
                }
                uudnl=i+2; //update 0x0D0A pos
                uudlcount++;
            }
            else if (uuds==2 && (c>=32 && c<=96)) {
                state=INFO; 
                if (uuc==0 && c==96) uuc=1;
            } // some files use char 96, set for info;
            else if (uuds==2) {
                uuds=0;   state=NONE; 
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

dType uueParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int uueParser::TypeCount() {
    return 1;
}

void uueParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=0;
    uuds=uuds1=uudp=uudslen=uudh=0;
    uudstart=uudend=uudline=uudnl=uudlcount=uuc=0;
    info=i=inSize=0;
}
void uueParser::SetEnd(uint64_t e) {
    jend=e;
}
