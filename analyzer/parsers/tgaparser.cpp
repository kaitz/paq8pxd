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
            tga=i,tgax=tgay,tgaz=8,tgat=(buf1>>8)&0xF;
            total=0; isPal=false;
        } else if (state==START) {
            if (i-tga==8) tga=(buf1==0?tga:0),tgax=(bswap(buf0)&0xffff),tgay=(bswap(buf0)>>16);
            else if (i-tga==10) {
                if ((buf0&0xFFF7)==32<<8)
                tgaz=32;
                if ((tgaz<<8)==(int)(buf0&0xFFD7) && tgax && tgay && uint32_t(tgax*tgay)<0xFFFFFFF) {
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
                            info=IMAGE8GRAY<<24;
                        info|=tgax;
                        pinfo="(width: "+ itos(tgax) +")";
                        total=tgax*tgay, line=0;
                    }
                }
                tga=0;
            }
        } else if (state==INFO && isPal && i>=(jstart-palcolors)) { // fill pal buffer and test if grayscale
                    assert(pali<palcolors);
                    pal[pali++]=c;
                    if (pali==palcolors) {
                        tgagray=IsGrayscalePalette(&pal[0], palcolors/3);
                        isPal=false;
                        palcolors=pali=0;
                        if (tgagray) info=tgax|(IMAGE8GRAY<<24);
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

bool TGAParser::IsGrayscalePalette(uint8_t *palb, int n, int isRGBA) {
  int stride = 3+isRGBA, res = (n>0)<<8, order=1;
  int l=0;
  for (int i = 0; (i < n*stride) && (res>>8); i++) {
    int b = palb[l++];
    if (l>n*3) {
      res = 0;
      break;
    }
    if (!i) {
      res = 0x100|b;
      order = 1-2*(b>int(ilog2(n)/4));
      continue;
    }

    //"j" is the index of the current byte in this color entry
    int j = i%stride;
    if (!j){
      // load first component of this entry
      int k = (b-(res&0xFF))*order;
      res = res&((k>=0 && k<=8)<<8);
      res|=(res)?b:0;
    }
    else if (j==3)
      res&=((!b || (b==0xFF))*0x1FF); // alpha/attribute component must be zero or 0xFF
    else
      res&=((b==(res&0xFF))*0x1FF);
  }
  return (res>>8)>0;
}

dType TGAParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
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
}
