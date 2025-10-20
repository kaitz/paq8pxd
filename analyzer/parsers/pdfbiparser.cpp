#include "pdfbiparser.hpp"

pdfBiParser::pdfBiParser() {
    priority=2;
    Reset();
    inpos=0;
    name="pdf bitmap";
}

pdfBiParser::~pdfBiParser() {
}

// loop over input block byte by byte and report state
DetectState pdfBiParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<128) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos/* || pos==0*/) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;
//if (state==START) printf("%c",c);

        if (state==NONE && buf0==0x42490D0A) { // 'BI\r\n'
            state=START;
            pdfi1=i,pdfi_ptr=pdfiw=pdfih=pdfic=pdfin=0;
        } else if (state==START) {
            if (pdfi1) {
                if (pdfi_ptr) {
                    int s=0;
                    if ((buf0&0xffffff)==0x2F5720) pdfi_ptr=0, pdfin=1; // /W 
                    if ((buf0&0xffffff)==0x2F4820 ) pdfi_ptr=0, pdfin=2; // /H
                    if ((buf1&0xff)==0x2F && buf0==0x42504320) pdfi_ptr=0, pdfin=3; // /BPC
                    if (buf1==0x2F494D20 && buf0==0x74727565) pdfi_ptr=0, pdfin=4; // /IM
                    if ((buf0&0xffffff)==0x435320) pdfi_ptr=0, pdfin=-1; // CS
                    if ((buf0&0xffffff)==0x494420) pdfi_ptr=0, pdfin=5; // ID
                    if (c==0x0a) {
                        if (pdfin==0) pdfi1=0;
                        else if (pdfin>0 && pdfin<4) s=pdfin;
                        if (pdfin==-1) pdfi_ptr=0;
                        if (pdfin!=5) pdfin=0;
                        
                    }
                    if (s) {
                        if (pdfi_ptr>=16) pdfi_ptr=16;
                        pdfi_buf[pdfi_ptr++]=0;
                        int v=atoi(&pdfi_buf[0]);
                        if (v<0 || v>1000) v=0;
                        if (s==1) pdfiw=v; else if (s==2) pdfih=v; else if (s==3) pdfic=v; else if (s==4) { };
                        if (v==0 || (s==3 && v>255)) pdfi1=0; else pdfi_ptr=0;
                    }
                }
                pdfi_buf[pdfi_ptr++]=((c>='0' && c<='9') || ' ')?c:0;
                if (pdfi_ptr>=16) pdfi1=pdfi_ptr=0;
                if (i-pdfi1>63) pdfi1=pdfi_ptr=0;
                if (pdfiw && pdfih && pdfic==1 && pdfin==5){

                    info=(pdfiw+7)/8;
                    jstart=i-pdfi1+4;
                    jend=jstart+pdfih*info;
                    pdfi1=pdfi_ptr=0;
                    type=IMAGE1;
                    state=END;
                    return state;
                } //IMG_DETP(IMAGE1,pdfi1-3,i-pdfi1+4,(pdfiw+7)/8,pdfih);
                if (pdfiw && pdfih && pdfic==8 && pdfin==5) {
                    info=pdfiw;
                    jstart=i-pdfi1+4;
                    jend=jstart+pdfih*info;
                    
                    type=IMAGE8;
                    state=END;
                    return state;
                } //IMG_DETP(IMAGE8,pdfi1-3,i-pdfi1+4,pdfiw,pdfih);
                //#define IMG_DETP(type,start_pos,header_len,width,height) return dett=(type),
                //deth=int(header_len),detd=int((width)*(height)),info=int(width),
                //in->setpos(start+(start_pos)),TEXT
            }
        } /*else if (state==INFO && i==(jend-1)) {
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

dType pdfBiParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int pdfBiParser::TypeCount() {
    return 1;
}

void pdfBiParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    pdfi1=pdfiw=pdfih=pdfic=0;
    //memset(&pdfi_buf[0], 0, 32); 
    pdfi_ptr=pdfin=0;
    info=i=inSize=0;
}
void pdfBiParser::SetEnd(uint64_t e) {
    jend=e;
}
