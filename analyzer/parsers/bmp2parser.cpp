#include "bmp2parser.hpp"

BMP2Parser::BMP2Parser() {
    priority=2;
    Reset();
    inpos=0;
    name="bmp os2";
}

BMP2Parser::~BMP2Parser() {
}

// loop over input block byte by byte and report state
DetectState BMP2Parser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
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
        if (state==NONE && (((buf0&0xffff)==16973)) ){
            state=START;
            bmp=int64_t(i);
        } else if (state==START) {
            int p=i-bmp;
            if (p==12) {
                of=bswap(buf0);
                if (!(of==26 || of==794)) state=NONE;
            }
            else if (p==16 && buf0==0x28000000) state=NONE;
            else if (p==18) x=bswap16(buf0&0xffff),bmp=((x==0||x>0x30000)?0:bmp); //width
            else if (p==20) y=abs(int32_t(bswap16(buf0&0xffff))),bmp=((y==0||y>0x10000)?0:bmp); //height
            else if (p==24) bpp=bswap16(buf0&0xffff),bmp=((bpp!=8 && bpp!=24)?0:bmp);
            else if (p==22 && int32_t(bswap16(buf0&0xffff))!=1) state=NONE;
            else if (p==4) size=bswap(buf0);
            else if (p==25) {
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
                    if (type==IMAGE8){
                        type=BM8_IMG;
                        TCOLORS=256;
                        for (int j=0; j<TCOLORS; j++) {
                            ColorRGBA colori;
                            uint32_t c=data[inSize++];
                            c=c*256+data[inSize++];
                            c=c*256+data[inSize++];
                            c=c*256;
                            colori.c=c;
                            colori.i=j;
                            bmcolor.push_back(colori);
                        }
                        RGBSort3D(bmcolor);
                        // Map to new order
                        for (int i=0; i<TCOLORS; ++i) {
                            for (int j=0; j<TCOLORS; ++j) {
                                ColorRGBA colori=bmcolor[j];
                                if (colori.i==i) {
                                    pal[i]=j;
                                    break;
                                } 
                            }
                        }
                        bmcolor.clear();
                        //if (type==BM8_IMG) 
                        info=info+(BM8_OS2<<24);
                    }
                    pinfo="(width: "+ itos(info&0xffffff) +") OS/2";
                    // report state only if larger then block
                    if ((jend-jstart)>len) return state;
                    else if ((jend-jstart)<len) {
                        state=END;
                        return state;
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
    if (state==INFO && (i-jstart)>64000) { priority=0; return INFO;}
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType BMP2Parser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    t.sData=&pal[0];
    return t;
}

void BMP2Parser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    bmp=bpp=x=y=of=size=info=i=inSize=0;
    priority=2;
    TCOLORS=256;
    bmcolor.reserve(TCOLORS);
}
