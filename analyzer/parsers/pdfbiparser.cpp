#include "pdfbiparser.hpp"
// Page 315-318, Adobe Portable Document Format, Version 1.5
// :inline image
pdfBiParser::pdfBiParser() {
    priority=3;
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

        if (state==NONE && buf0==0x42490D0A) { // 'BI\r\n'
            state=START;
            pdfi1=i,pdfi_ptr=pdfiw=pdfih=pdfic=pdfin=0;
        } else if (state==START) {
            if (pdfi1) {
                // Single line header with /F/CCF (CCITTFaxDecode)?
                if (pdfi_ptr) { // multiline header detection, /F flag not tested
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
                pdfi_buf[pdfi_ptr++]=((c>='0' && c<='9') ||c==' ')?c:0;
                if (pdfi_ptr>=16) pdfi1=pdfi_ptr=0;
                if (i-pdfi1>63) pdfi1=pdfi_ptr=0;
                if (pdfiw && pdfih && pdfic==1 && pdfin==5){
                    info=(pdfiw+7)/8;
                    jstart=i+1;
                    jend=jstart+pdfih*info;
                    pdfi1=pdfi_ptr=0;
                    if (pdfiw>1) {
                        type=IMAGE1;
                        state=END;
                        pinfo="(width: "+ itos(info) +")";
                        return state;
                    } else {
                        state=NONE;
                    }
                }
                if (pdfiw && pdfih && pdfic==8 && pdfin==5) {
                    info=pdfiw;
                    jstart=i+1;
                    jend=jstart+pdfih*info;
                    pinfo="(width: "+ itos(info) +")";
                    type=IMAGE8;
                    state=END;
                    return state;
                }
            }
        }

        inSize++;
        i++;
    }
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType pdfBiParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void pdfBiParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    pdfi1=pdfiw=pdfih=pdfic=0;
    //memset(&pdfi_buf[0], 0, 32); 
    pdfi_ptr=pdfin=0;
    info=i=inSize=0;
    priority=3;
}
