#include "gifparser.hpp"

GIFParser::GIFParser() {
    priority=3;
    Reset();
    inpos=0;
    name="gif";
}

GIFParser::~GIFParser() {
}

void GIFParser::ReorderPal(unsigned char *data){
    TCOLORS=plt/3;
    pals=i+1;
    int i=0;
    for (int j=0; j<TCOLORS*3; j=j+3) {
        ColorRGBA colori;
        uint32_t c=data[inSize+1+j];
        c=c*256+data[inSize+1+j+1];
        c=c*256+data[inSize+1+j+2];
        c=c*256+0;
        colori.c=c;
        colori.i=i++;
        bmcolor.push_back(colori);
    }
    // fill empty
    for (int j=TCOLORS*3; j<256*3; j=j+3) {
        ColorRGBA colori;
        uint32_t c=0xffffffff;
        colori.c=c;
        colori.i=i++;
        bmcolor.push_back(colori);
    }
    RGBSort3D(bmcolor);
    int TCOLORS1=256;
    // Map to new order
    for (int i=0; i<TCOLORS1; ++i) {
        for (int j=0; j<TCOLORS1; ++j) {
            ColorRGBA colori=bmcolor[j];
            if (colori.i==i) {
                palo[i]=j;
                break;
            } 
        }
    }
    bmcolor.clear();
}

// loop over input block byte by byte and report state
DetectState GIFParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<32) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos || pos==0) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        int c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (buf1&0xffff)==0x4749 && (buf0==0x46383961 || buf0==0x46383761) ){
            state=START;
            gif=1,gifi=i+5;
        } 
        if (state==START || state==INFO) {
            if (gif==1 && i==gifi) gif=2,gifi = i+5+(plt=(c&128)?(3*(2<<(c&7))):0);
            if (gif==2 && plt && i==gifi-plt-3) {
                ReorderPal(data);
                gray = IsGrayscalePalette(&data[inSize+1], plt/3), plt = 0;
            }
            if (gif==2 && i==gifi) {
                if ((buf0&0xff0000)==0x210000) gif=5,gifi=i;
                else if ((buf0&0xff0000)==0x2c0000) gif=3,gifi=i,state=INFO;
                else gif=0,state=NONE;
            }
            if (gif==3 && i==gifi+6) gifw=(bswap(buf0)&0xffff);
            if (gif==3 && i==gifi+7) gif=4,gifc=gifb=0,gifa=gifi=i+2+(plt=((c&128)?(3*(2<<(c&7))):0));
            if (gif==4 && plt) gray = IsGrayscalePalette(&data[inSize+1], plt/3),ReorderPal(data), plt = 0;
            if (gif==4 && i==gifi) {
                if (c>0 && gifb && gifc!=gifb) gifw=0;
                if (c>0) gifb=gifc,gifc=c,gifi+=c+1;
                else if (!gifw) gif=2,gifi=i+3;
                else {
                    jstart=gifa-1;
                    jend=i+1;
                    state=END;
                    if (rec==false) info=((jstart-pals))*1024+TCOLORS,info<<=32; else info=0; // only for the first image
                    info|=((gray?IMAGE8GRAY:IMAGE8)<<24)|gifw;
                    pinfo="(width: "+ itos(gifw) +")";
                    type=GIF;
                    if (data[inSize+1]==0x2c || data[inSize+1]==0x21) rec=true;
                    else rec=false;
                    return state;
                }
            }
            if (gif==5 && i==gifi) {
                if (c>0) gifi+=c+1; else gif=2,gifi=i+3;
            }
            if (state==NONE) rec=false;
        }
        if (state==END) {
            // Animated?
            state=START;
            if (c==0x2c || c==0x21) gif=2,gifi=2;
            else gray=0;
            
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

dType GIFParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=rec;
    t.sData=&palo[0];
    return t;
}

void GIFParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    gif=gifa=gifi=gifw=gifc=gifb=plt=gray=0;
    info=i=inSize=0;
    rec=false;
    priority=3;
    TCOLORS=256;
    bmcolor.reserve(TCOLORS);
}
