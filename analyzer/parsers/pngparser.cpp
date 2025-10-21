#include "pngparser.hpp"

PNGParser::PNGParser() {
    priority=1;
    Reset();
    inpos=0;
    name="png";
}

PNGParser::~PNGParser() {
}

// loop over input block byte by byte and report state
DetectState PNGParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<128) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf3=(buf3<<8)|(buf2>>24);
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (buf3==0x89504E47 && buf2==0x0D0A1A0A && buf1==0x0000000D && buf0==0x49484452) ){
            state=START;
            png=i, pngtype=-1, lastchunk=buf3;
        } else if (state==START || state==INFO) {
            if (png){
                const int p=i-png;
                if (p==12){
                    pngw = buf2;
                    pngh = buf1;
                    pngbps = buf0>>24;
                    pngtype = (uint8_t)(buf0>>16);
                    pnggray = 0;
                    png*=((buf0&0xFFFF)==0 && pngw && pngh && pngbps==8 && (!pngtype || pngtype==2 || pngtype==3 || pngtype==4 || pngtype==6));
                    
                    //printf("PNG %d W:%d H:%d B:%d T:%d\n ",png,pngw,pngh,pngbps,pngtype);
                } else if (p>12 && pngtype<0){
                    png = 0; state=NONE;    
                } else if (p==17){
                    png*=((buf1&0xFF)==0);
                    nextchunk =(png)?i+8:0;
                } else if (p>17 && i==nextchunk){state=INFO;
                    nextchunk+=buf1+4+8;//CRC, Chunk length+Id
                    lastchunk = buf0;
                    if (lastchunk==0x504C5445){//PLTE
                        png*=(buf1%3==0);
                        //pnggray = (png && IsGrayscalePalette(in, buf1/3));
                    } else if (lastchunk==0x49444154){ // IDAT
                        //printf("Size: %d\n",buf1);
                        jstart=i+1;
                        jend=jstart+buf1;
                        type=ZLIB;
                        info=0;
                        if (pngw<0x1000000 && pngh) {//IDAT
                            if (pngbps==8 && pngtype==2) info=(PNG24<<24)|(pngw*3);
                            else if (pngbps==8 && pngtype==6 ) info=(PNG32<<24)|(pngw*4);
                            else if (pngbps==8 && (!pngtype || pngtype==3)) info=(((!pngtype || pnggray)?PNG8GRAY:PNG8)<<24)|(pngw);
                            pinfo=" (width: "+ itos(info&0xffffff) +") "+(pngtype==2?"PNG24":(pngtype==6?"PNG32":"PNG8"));
                        }
                        
                    } else if (lastchunk==0x49454E44) { //IEND)
                      state=END;
                      return state;
                    }
                }
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

dType PNGParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int PNGParser::TypeCount() {
    return 1;
}

void PNGParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=buf3=0;
    png=lastchunk=nextchunk=0;               // For PNG detection
    pngw=pngh=pngbps=pngtype=pnggray=0;
    info=i=inSize=0;
}
void PNGParser::SetEnd(uint64_t e) {
    jend=e;
}
