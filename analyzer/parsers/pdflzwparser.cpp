#include "pdflzwparser.hpp"

PDFLzwParser::PDFLzwParser() {
    priority=3;
    Reset();
    inpos=0;
    name="pdf lzw";
}

PDFLzwParser::~PDFLzwParser() {
}

// loop over input block byte by byte and report state
DetectState PDFLzwParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<256) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf4=(buf4<<8)|(buf3>>24);
        buf3=(buf3<<8)|(buf2>>24);
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (buf1&0xffffff)==0x2F5769 && buf0==0x64746820) { // '/Width '
            imagePos=i; width=0;
        } else if (imagePos) {
            if (c>='0' && c<='9') {
                width*=10;
                width+=c-'0';
            } else if (width) imagePos=0;
        } 
        if (state==NONE && buf1==0x2F486569  && buf0==0x67687420) { // '/Height '
            heightPos=i; height=0;
        } else if (heightPos) {
            if (c>='0' && c<='9') {
                height*=10;
                height+=c-'0';
            } else if (height) heightPos=0;
        }  
        if (state==NONE && (buf4&0xffff)==0x2F42 && buf3==0x69747350 && buf2==0x6572436F && buf1==0x6D706F6E && buf0==0x656E7420) { // '/BitsPerComponent '
            bitsPos=i; bits=0;
        } else if (bitsPos) {
            if (c>='0' && c<='9') {
                bits*=10;
                bits+=c-'0';
            } else if (bits) bitsPos=0;
        }
        if (state==NONE && (buf2&0xffffff)==0x2F4465  && buf1==0x76696365 && buf0==0x47726179) { // ' /DeviceGray'
            grayPos=i;
        }
        if ((state==NONE || state==START) && buf1==0x2F4C656E && buf0==0x67746820) { // '/Length '
            // This field can contain wrong values (large diff to real lenght)
            // When it is off by 1 byte then (endstream-stream)-lengh=1 
            // then lenght may have correct value.
            lenghtPos=i;
            lenght=0;
        } else if (lenghtPos) {
            if (c>='0' && c<='9') {
                lenght*=10;
                lenght+=c-'0';
            } else if (lenght) lenghtPos=0;
        }
        if (state==NONE && (
                    (buf4==0x2F4C5A57 && buf3==0x4465636F && buf2==0x64650D0A && buf1==0x3E3E0D0A && buf0==0x73747265) ||          // '/LZWDecode\r\n>>\r\nstre'
                    (buf4==0x202F4669  && buf3==0x6C746572  && buf2==0x202F4C5A  && buf1==0x57446563  && buf0==0x6F646520) ||      // ' /Filter /LZWDecode '
                    ((buf4&0xff)==0x2F && buf3==0x46696C74 && buf2==0x65722F4A && buf1==0x50584465 && buf0==0x636F6465) ||         // '/Filter/JPXDecode'
                    ((buf4&0xffffff)==0x2F4669 && buf3==0x6C746572 && buf2==0x2F4A4249 && buf1==0x47324465 && buf0==0x636F6465) || // '/Filter/JBIG2Decode'
                    (buf4==0x2F46696C && buf3==0x74657220 && buf2==0x2F466C61 && buf1==0x74654465 && buf0==0x636F6465)   ||        // '/Filter /FlateDecode'
                    ((buf4&0xffffff)==0x2F4669 && buf3==0x6C746572 && buf2==0x2F466C61 && buf1==0x74654465 && buf0==0x636F6465)    // '/Filter/FlateDecode'
                    )) {
            state=START;
            pLzwp=i;
            if (buf1==0x57446563 || buf4==0x2F4C5A57) pinfo="PDF LZW";
            else if (buf1==0x50584465) pinfo="PDF JPX";
            else if (buf1==0x47324465) pinfo="PDF JBIG2";
            if (buf1==0x74654465) {
                    //printf("IMG %d x %d %d bits.",width,height, bits);
                    type=ZLIB;
                    pinfo="PDF ZLIB";
                    if (width>1 && width<0x1000000 && height>1) {
                        int pdfgray=0;
                        if ((pLzwp-grayPos)<100) pdfgray=1;
                        if (bits==8) pinfo="8/24b image",info=((pdfgray?IMAGE8GRAY:IMAGE8)<<24)|width;
                        else if (bits==4 ) pinfo="4b image",info=(IMAGE4<<24)|((width+1)/2);
                        else if (bits==1 ) pinfo="1b image",info=(IMAGE1<<24)|((width+7)/8);
                        if (info) pinfo+=" (width: "+ itos(info&0xffffff) +")",info|=uint64_t(height)<<32;
                    }
            }
            imagePos=0;
        } else if (state==START) {
            if (buf0==0x616D0D0A || buf0==0x65616D0A) { // 'am\r\n' 'eam\n'
                jstart=i+1;
                pLzwp=jstart-pLzwp;
                if (pLzwp>=5 && pLzwp<150) { // LZWDecode and stream distance in 10-150 bytes
                    state=INFO;    
                } else {
                    state=NONE;
                }
            }
        } else if (state==INFO /*&& lenght*/) {
            if (buf1==0x0D656E64 && buf0==0x73747265 || buf1==0x0A656E64 && buf0==0x73747265 ) { // '\rendstre' '\nendstre'
                jend=i-8+1;
                jend=jend-(buf1==0x0A656E64 && ((buf2&255)==0x0D)?1:0); // of by one?
                uint32_t len=jend-jstart;
                if ((len-lenght)!=0 && type==ZLIB) {
                    //jend=jstart+lenght;   // is lenght correct?
                    state=NONE;
                } else {
                    if (type!=ZLIB) type=CMP;
                    state=END;
                    return state;
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

dType PDFLzwParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void PDFLzwParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=buf3=buf4=0;
    pLzwp=0;
    info=i=inSize=imagePos=lenghtPos=heightPos=bitsPos=grayPos=0;
    lenght=width=height=bits=0;
    priority=3;
    pinfo="";
}
