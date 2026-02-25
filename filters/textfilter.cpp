#include "textfilter.hpp"
//Based on XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski
#include "wrtpre.inc"

TextFilter::TextFilter(std::string n, Settings &s, Filetype f):Filter(s) {
    name=n;
    Type=f;
}

void TextFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    int wrtn=int(info&0xffffffff);
    assert(wrtn<2);
    XWRT_Encoder* wrt;
    wrt=new XWRT_Encoder(settings);
    wrt->defaultSettings(wrtn);
    wrt->WRT_start_encoding(in,out,size,false);
    delete wrt;
} 

uint64_t TextFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    XWRT_Decoder* wrt;
    wrt=new XWRT_Decoder(settings);
    wrt->defaultSettings(0);
    uint64_t bb=wrt->WRT_start_decoding(in);
    for (uint64_t i=0L; i<bb; i++) {
        out->putc(wrt->WRT_decode());
    }
    delete wrt;
    fsize=bb;
    return bb; 
}

TextFilter::~TextFilter() {
}

