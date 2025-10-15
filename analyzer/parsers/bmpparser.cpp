#include "bmpparser.hpp"

BMPParser::BMPParser() {
    priority=2;
    Reset();
    inpos=0;
    name="bmp";
}

BMPParser::~BMPParser() {
}

// loop over input block byte by byte and report state
DetectState BMPParser::Parse(unsigned char *data, uint64_t len, uint64_t pos) {
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
        // Detect for 'BM' or headerless DIB
        if (state==NONE && (((buf0&0xffff)==16973) || (!(buf0&0xFFFFFF) && ((buf0>>24)==0x28))) ){
            state=START;
            hdrless=!(buf0&0xff),of=hdrless*54;
            bmp=i-hdrless*16;
        } else if (state==START) {
            int p=i-bmp;
            if (p==12) of=bswap(buf0);
            else if (p==16 && buf0!=0x28000000) state=NONE;//BITMAPINFOHEADER (0x28)
            else if (p==20) x=bswap(buf0),bmp=((x==0||x>0x30000)?(hdrless=0):bmp); //width
            else if (p==24) y=abs(bswap(buf0)),bmp=((y==0||y>0x10000)?(hdrless=0):bmp); //height
            else if (p==27) bpp=c,bmp=((bpp!=1 && bpp!=4 && bpp!=8 && bpp!=24 && bpp!=32)?(hdrless=0):bmp);
            else if ((p==31) && buf0) state=NONE;
            else if (p==36) size=bswap(buf0);
            // check number of colors in palette (4 bytes), must be 0 (default) or <= 1<<bpp.
            // also check if image is too small, since it might not be worth it to use the image models
            else if (p==48) {
                if ( (!buf0 || ((bswap(buf0)<=(1<<bpp)) && (bpp<=8))) && (((x*y*bpp)>>3)>64) ) {
                    // possible icon/cursor?
                    if (hdrless && (x*2==y) && bpp>1 && (
                                (size>0 && size==( (x*y*(bpp+1))>>4 )) ||
                                ((!size || size<((x*y*bpp)>>3)) && (
                                        (x==8)   || (x==10) || (x==14) || (x==16) || (x==20) ||
                                        (x==22)  || (x==24) || (x==32) || (x==40) || (x==48) ||
                                        (x==60)  || (x==64) || (x==72) || (x==80) || (x==96) ||
                                        (x==128) || (x==256)
                                        ))
                                )
                            )
                    y=x;
                    // if DIB and not 24bpp, we must calculate the data offset based on BPP or num. of entries in color palette
                    if (hdrless && (bpp<24))
                        of=of+((buf0)?bswap(buf0)*4:4<<bpp);
                    of=of+(bmp-1)*(bmp<1);
                    if (hdrless && size && size<((x*y*bpp)>>3)) { }//Guard against erroneous DIB detections
                    else { 
                        if (bpp==24) {
                            info=((x*3)+3)&0xFFFFFFFC;// -4;
                            type=IMAGE24;
                        } else  if (bpp==4) {
                            info=((x*4+31)>>5)*4;
                            type=IMAGE4;
                        } else  if (bpp==8) {
                            info=(x+3)&0xFFFFFFFC;// -4;
                            type=IMAGE8;
                        } else  if (bpp==1) {
                            info=(((x-1)>>5)+1)*4;
                            type=IMAGE1;
                        }
                        if (type!=DEFAULT){
                            state=INFO;
                            jstart=of+bmp-1;
                            jend=info*y+jstart;
                            // report state only if larger then block
                            if ((jend-jstart)>len) return state;
                        }
                    }
                }
            }
            if (p>48) state=NONE;
        } else if (state==INFO && i==(jend-1)) {
            state=END;
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

dType BMPParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int BMPParser::TypeCount() {
    return 1;
}

void BMPParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    bmp=bpp=x=y=of=size=hdrless=info=i=inSize=0;
}
void BMPParser::SetEnd(uint64_t e) {
    jend=e;
}
