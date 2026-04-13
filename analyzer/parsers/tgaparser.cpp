#include "tgaparser.hpp"

TGAParser::TGAParser() {
    priority=3;
    Reset();
    inpos=0;
    name="tga";
}

TGAParser::~TGAParser() {
}

// loop over input block byte by byte and report state
DetectState TGAParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
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

        if (state==NONE && (buf1&0xFFF7FF)==0x00010100 && (buf0&0xFFFFFFC7)==0x00000100 && (c==16 || c==24 || c==32) ){
            state=START;
            tga=i,tgax=tgay,tgaz=8,tgat=(buf1>>8)&0xF,tgaid=buf1>>24,tgamap=c/8;
            total=0; isPal=false;
        } else if (state==NONE && (buf1&0xFFFFFF)==0x00000200 && buf0==0x00000000) {
            state=START;
            tga=i,tgax=tgay,tgaz=24,tgat=2;
            total=0; isPal=false;
        } else if (state==NONE && (buf1&0xFFF7FF)==0x00000300 && buf0==0x00000000) {
            state=START;
            tga=i,tgax=tgay,tgaz=8,tgat=(buf1>>8)&0xF;tgaid=buf1>>24;
            total=0; isPal=false;
        } else if (state==START) {
            if (i-tga==8) tga=(buf1==0?tga:0),tgax=(bswap(buf0)&0xffff),tgay=(bswap(buf0)>>16);
            else if (i-tga==10) {
                if ((buf0&0xFFF7)==32<<8)
                tgaz=32;
                if ((tgaz<<8)==(int)(buf0&0xFFD7) && tgax && tgay && uint32_t(tgax*tgay)<0xFFFFFFF) {
                    for (int j=0; j<256; ++j) palo[i]=i;
                    if (tgat==1) {
                        jstart=tga-7+18+tgaid+256*tgamap;
                        jend=jstart+tgax*tgay;
                        info=tgay;
                        state=INFO;
                        type=IMAGE8;
                        pinfo="(width: "+ itos(info) +")";
                    } else if (tgat==2) {
                        jstart=tga-7+18+tgaid;
                        jend=jstart+tgax*(tgaz>>3)*tgay;
                        info=tgay;
                        state=INFO;
                        type=(tgaz==24)?IMAGE24:IMAGE32;
                        pinfo="(width: "+ itos(info) +")";
                    } else if (tgat==3) {
                        jstart=tga-7+18+tgaid;
                        jend=jstart+tgax*tgay;
                        info=tgay;
                        state=INFO;
                        type=IMAGE8GRAY;
                        pinfo="(width: "+ itos(info) +")";
                    } else if (tgat==9 || tgat==11) {
                        state=INFO;
                        if (tgat==9) {
                            info=IMAGE8<<24;
                            palcolors=tgamap*256;
                            jstart=tga-7+18+tgaid+palcolors;
                            isPal=true;
                            pali=0;
                        }
                        else
                        info=IMAGE8GRAY<<24,jstart=tga-7+18+tgaid;
                        info|=tgax;
                        pinfo="(width: "+ itos(tgax) +")";
                        total=tgax*tgay, line=0;
                    }
                }
                tga=0;
            }
        } else if (state==INFO && isPal && i>=(jstart-palcolors) && palcolors) { // fill pal buffer and test if grayscale
            assert(pali<palcolors);
            pal[pali++]=c;
            if (pali==palcolors) {
                tgagray=IsGrayscalePalette(&pal[0], palcolors/3);
                isPal=false;
                palcolors=pali=0;
                if (tgagray) info=tgax|(IMAGE8GRAY<<24);
                TCOLORS=256;
                int i=0;
                for (int j=0; j<TCOLORS*3; j=j+3) {
                    ColorRGBA colori;
                    uint32_t c=pal[j+0];
                    c=c*256+pal[j+1];
                    c=c*256+pal[j+2];
                    c=c*256+0;
                    colori.c=c;
                    colori.i=i++;
                    bmcolor.push_back(colori);
                }
                RGBSort3D(bmcolor);
                // Map to new order
                for (int i=0; i<TCOLORS; ++i) {
                    for (int j=0; j<TCOLORS; ++j) {
                        ColorRGBA colori=bmcolor[j];
                        if (colori.i==i) {
                            palo[i]=j;
                            break;
                        } 
                    }
                }
                bmcolor.clear();
            }
        } else if (state==INFO && total && i>=jstart) {
            // Detect RLE compressed image data size
            if (rlep) {
                rlep--;
            } else {
                if (c==0x80) { ; }
                else if (c>0x7F) { //  run 
                    total-=(c=(c&0x7F)+1); rlep=1; line+=c;
                } else {           // no run
                    c++; rlep=c; total-=c; line+=c;
                }
                if (line>tgax) state=NONE;
                else if (line==tgax) line=0;
            }
            if (total==0) {
                type=RLE;
                jend=i+2;
            } else if (total<0) {
                state=NONE;
            }
        } else if (state==INFO && i==(jend-1)) {
            state=END;
            return state;
        }

        inSize++;
        i++;
    }
    // Are we still reading data for our type
    if (state==INFO) return INFO;
    else if (state!=NONE)
    return DATA;
    else return NONE;
}

dType TGAParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    t.sData=&palo[0];
    return t;
}

void TGAParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    tga=0;
    tgax=0;
    tgay=tgaz=tgat=tgaid=tgamap=0;
    info=i=inSize=0;
    total=line=rlep=0;
    palcolors=0;
    pali=0;
    isPal=false;
    priority=3;
    TCOLORS=256;
    bmcolor.reserve(TCOLORS);
}
