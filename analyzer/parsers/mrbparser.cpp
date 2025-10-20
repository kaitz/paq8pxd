#include "mrbparser.hpp"

uint32_t mrbParser::GetCDWord(unsigned char *data ){
    uint16_t w = data[inSize];inSize++; i++;
    w=w | (data[inSize]<<8);inSize++; i++;
    if(w&1){
        uint16_t w1 = data[inSize];inSize++; i++;
        w1=w1 | (data[inSize]<<8);inSize++; i++;
        return ((w1<<16)|w)>>1;
    }
    return w>>1;
}
uint16_t mrbParser::GetCWord(unsigned char *data ){
    uint16_t b=data[inSize];inSize++; i++;
    if(b&1) {i++;return ((data[inSize++]<<8)|b)>>1;
    }
    return b>>1;
}
uint8_t mrbParser::GetC(unsigned char *data ){
    i++;
    return data[inSize++];
}

mrbParser::mrbParser() {
    priority=2;
    Reset();
    inpos=0;
    name="mrb";
}

mrbParser::~mrbParser() {
}

// loop over input block byte by byte and report state
DetectState mrbParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
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
        if (state==NONE && ((buf0&0xffFFFF)==0x00006c70 || (buf0&0xFFFF)==0x6C50) ){
            state=START;
            mrb=i,mrbsize=0,mrbPictureType=mrbmulti=0; // TODO test if not end of the block
        } 
        else if (state==START) {
            const uint64_t p=(i-mrb)-mrbmulti*4; // Select only first image from multiple
            if (p==1 && c>1 && c<4&& mrbmulti==0) mrbmulti=c-1;
            else if (p==1 && c==0) state=NONE,mrb=0;
            else if (p==6) poffset=bswap(buf0);  // PictureOffset
            else if (p==7) {
                // 5=DDB 6=DIB 8=metafile
                if (/*c==5 ||*/ c==6) mrbPictureType=c;
                else state=NONE,mrb=0;
            } else if (p==8) {         // 0=uncomp 1=RunLen 2=LZ77 3=both
                if (c==1 ||/*c==2||c==3||*/c==0) mrbPackingMethod=c;
                else state=NONE,mrb=0;
            } else if (p==9){
                if (mrbPictureType==6 && (mrbPackingMethod==0 || mrbPackingMethod==1 || mrbPackingMethod==2)){
                    //save ftell
                    mrbTell=i;
                    uint32_t Xdpi=GetCDWord(data);
                    uint32_t Ydpi=GetCDWord(data);
                    uint32_t Planes=GetCWord(data);
                    uint32_t BitCount=GetCWord(data);
                    mrbw=GetCDWord(data);
                    mrbh=GetCDWord(data);
                    uint32_t ColorsUsed=GetCDWord(data);
                    uint32_t ColorsImportant=GetCDWord(data);
                    mrbcsize=GetCDWord(data); //CompressedSize
                    uint32_t HotspotSize=GetCDWord(data);
                    int CompressedOffset=(GetC(data)<<24)|(GetC(data)<<16)|(GetC(data)<<8)|GetC(data);
                    int HotspotOffset=(GetC(data)<<24)|(GetC(data)<<16)|(GetC(data)<<8)|GetC(data);
                    CompressedOffset=bswap(CompressedOffset);
                    HotspotOffset=bswap(HotspotOffset);
                    if (mrbPackingMethod==1) mrbsize=mrbcsize+ i-mrbTell+10+(1<<BitCount)*4; // ignore HotspotSize
                    int pixelBytes = (mrbw * mrbh * BitCount) >> 3;
                    if (BitCount!=1 && BitCount!=4 && BitCount!=8)state=NONE,mrb=0;
                    if ((mrbPackingMethod==0 ||mrbPackingMethod==1) && mrb>2 ) {
                        type=DEFAULT;
                        jend=mrb-2;
                        state=END;
                        return state;
                    }
                    if ((mrbPackingMethod==0 ||mrbPackingMethod==1) && mrb==2) {
                        if (mrbPackingMethod==0)  {
                            jstart=mrb+7+CompressedOffset,type=IMAGE1;info=(((mrbw-1)>>5)+1)*4; 
                        }
                        else if (mrbPackingMethod==1)  {
                            jstart=mrb+7+CompressedOffset;
                            if (   BitCount==8)type=MRBR;      
                            if (   BitCount==4)type=MRBR4;     
                            info=(((mrbw+3)/4)*4)+(mrbh<<32);
                        }
                        jend=jstart+mrbcsize;   
                        state=END;
                        /*if ((jend-jstart)>len)*/ return state;
                    }else
                    state=NONE,mrb=0;
                } else mrbPictureType=mrb=mrbsize=0;
            }
            if (p>10) state=NONE,mrb=0;
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

dType mrbParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int mrbParser::TypeCount() {
    return 1;
}

void mrbParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    mrb=0,mrbsize=0,mrbcsize=0,mrbPictureType=0,mrbPackingMethod=0,mrbTell=0,mrbTell1=0,mrbw=0,mrbh=0;
    info=i=inSize=0;
}
void mrbParser::SetEnd(uint64_t e) {
    jend=e;
}
