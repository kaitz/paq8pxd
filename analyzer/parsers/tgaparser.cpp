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
        } else if (state==NONE && (buf1&0xFFFFFF)==0x00000200 && buf0==0x00000000) {
            state=START;
            tga=i,tgax=tgay,tgaz=24,tgat=2;
        } else if (state==NONE && (buf1&0xFFF7FF)==0x00000300 && buf0==0x00000000) {
            state=START;
            tga=i,tgax=tgay,tgaz=8,tgat=(buf1>>8)&0xF;   
        } else if (state==START) {
            if (i-tga==8) tga=(buf1==0?tga:0),tgax=(bswap(buf0)&0xffff),tgay=(bswap(buf0)>>16);
            else if (i-tga==10) {
                if ((buf0&0xFFF7)==32<<8)
                tgaz=32;
                if ((tgaz<<8)==(int)(buf0&0xFFD7) && tgax && tgay && uint32_t(tgax*tgay)<0xFFFFFFF) {
                    if (tgat==1){
                        jstart=tga-7+18+tgaid+256*tgamap;
                        jend=jstart+tgax*tgay;
                        info=tgay;
                        state=END;
                        type=IMAGE8;
                        pinfo=" (width: "+ itos(info) +")";
                        return state;
                    } else if (tgat==2) {
                        jstart=tga-7+18+tgaid;
                        jend=jstart+tgax*(tgaz>>3)*tgay;
                        info=tgay;
                        state=END;
                        type=(tgaz==24)?IMAGE24:IMAGE32;
                        pinfo=" (width: "+ itos(info) +")";
                        return state;
                    } else if (tgat==3) {
                        jstart=tga-7+18+tgaid;
                        jend=jstart+tgax*tgay;
                        info=tgay;
                        state=END;
                        type=IMAGE8GRAY;
                        pinfo=" (width: "+ itos(info) +")";
                        return state;
                    } else if (tgat==9 || tgat==11) {
                        state=NONE;
                        /*const U64 savedpos=in->curpos();
                        in->setpos(start+tga+11+tgaid);
                        if (tgat==9) {
                            info=(IsGrayscalePalette(in)?IMAGE8GRAY:IMAGE8)<<24;
                            in->setpos(start+tga+11+tgaid+256*tgamap);
                        }
                        else
                        info=IMAGE8GRAY<<24;
                        info|=tgax;
                        // now detect compressed image data size
                        detd=0;
                        int c=in->getc(), b=0, total=tgax*tgay, line=0;
                        while (total>0 && c>=0 && (++detd, b=in->getc())>=0){
                            if (c==0x80) { c=b; continue; }
                            else if (c>0x7F) {
                                total-=(c=(c&0x7F)+1); line+=c;
                                c=in->getc();
                                detd++;
                            }
                            else {
                                in->setpos(in->curpos()+c); 
                                detd+=++c; total-=c; line+=c;
                                c=in->getc();
                            }
                            if (line>tgax) break;
                            else if (line==tgax) line=0;
                        }
                        if (total==0) {
                            in->setpos(start+tga+11+tgaid+256*tgamap);
                            return dett=RLE;
                        }
                        else
                        in->setpos(savedpos);*/
                    }
                }
                tga=0;
            }
        }/* else if (state==INFO && i==(jend-1)) {
            state=END;
            return state;
        }*/

        inSize++;
        i++;
    }
    // Are we still reading data for our type
    if (state!=NONE)
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
    return t;
}

void TGAParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    tga=0;
    tgax=0;
    tgay=tgaz=tgat=tgaid=tgamap=0;
    info=i=inSize=0;
    priority=3;
}
