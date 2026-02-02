#include "eolfilter.hpp"

//Based on XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski
 
EOLFilter::EOLFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
}

// simple color transform (b, g, r) -> (g, g-r, g-b)
void EOLFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    int len=int(size&0xffffffff);
    U64 eolz=0;
    U64 wrtz=0;
    FileTmp wrtfi;
    FileTmp tmpout;
    
    EOLEncoderCoder* eolc;
    eolc=new EOLEncoderCoder(&wrtfi);
    eolc->EOLencode(in,&tmpout,len); 
    out->put32(len);
    eolz= wrtfi.curpos();
    out->put32(eolz);
    
    wrtz= tmpout.curpos();
    out->put32(wrtz);
    wrtfi.setpos(0);
    for (U64 offset=0; offset<eolz; offset++) { 
        out->putc(wrtfi.getc()); 
   }
    wrtz= tmpout.curpos();
    tmpout.setpos(0);
    for (U64 offset=0; offset<wrtz; offset++) { 
        out->putc(tmpout.getc()); 
    }
    delete eolc;
    wrtfi.close();
    tmpout.close();
   // if (eolz<35) printf("Eol count %d<35\n",eolz);
    diffFound=eolz<35; // fake transform fail
} 

uint64_t EOLFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    int b=0;
    U64 bb=0;
    U64 eolz=0,wrtz=0;
    FileTmp wrtfi;
    FileTmp tmpout;
    int len=in->get32();
    eolz=in->get32();
    wrtz=in->get32();
    
    for (U64 offset=0; offset<eolz; offset++) wrtfi.putc(in->getc()); 
    wrtfi.setpos(0);
    EOLDecoderCoder* eold;
    eold=new EOLDecoderCoder(); 
    eold->EOLdecode(&tmpout,wrtz,&wrtfi,in,len);

    bb= tmpout.curpos();
    tmpout.setpos(0);
    for (U64 i=0; i<bb; i++) { // replace
        b=tmpout.getc();
        out->putc(b);
    }
    delete eold;
    tmpout.close();
    wrtfi.close();
    fsize=bb;
    return bb; 
}

EOLFilter::~EOLFilter() {
}

