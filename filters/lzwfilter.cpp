#include "lzwfilter.hpp"
// Needs test with proper working file.
lzwFilter::lzwFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
} 

void lzwFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    uint64_t len=size;
    LZWDictionary dic;
    int32_t parent=-1, code=0, buffer=0, bitsPerCode=9, bitsUsed=0;
    bool done = false;
    while (!done) {
        buffer=in->getc();
        if (buffer<0) { return; }
        for (int32_t j=0; j<8; j++) {
            code+=code+((buffer>>(7-j))&1), bitsUsed++;
            if (bitsUsed>=bitsPerCode) {
                if (code==LZW_EOF_CODE) { done=true; break;}
                else if (code==LZW_RESET_CODE) {
                    dic.reset();
                    parent=-1; bitsPerCode=9;
                } else {
                    if (code<dic.index) {
                        if (parent!=-1)
                        dic.addEntry(parent, dic.dumpEntry(out, code));
                        else
                        out->putc(code);
                    } else if (code==dic.index) {
                        int32_t a = dic.dumpEntry(out, parent);
                        out->putc(a);
                        dic.addEntry(parent,a);
                    }
                    else return ;
                    parent=code;
                }
                bitsUsed=0; code=0;
                if ((1<<bitsPerCode)==dic.index+1 && dic.index<4096)
                bitsPerCode++;
            }
        }
    }
}

inline void writeCode(File *f, int32_t *buffer, U64 *pos, int32_t *bitsUsed, const int32_t bitsPerCode, const int32_t code) {
    *buffer<<=bitsPerCode; *buffer|=code;
    (*bitsUsed)+=bitsPerCode;
    while ((*bitsUsed)>7) {
        const uint8_t B=*buffer>>(*bitsUsed-=8);
        (*pos)++;
        f->putc(B);
    }
}

uint64_t lzwFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    LZWDictionary dic;
    U64 pos=0;
    int32_t parent=-1, code=0, buffer=0, bitsPerCode=9, bitsUsed=0;
    writeCode(out, &buffer, &pos, &bitsUsed, bitsPerCode, LZW_RESET_CODE);
    while ((code=in->getc())>=0 && diffFound==0) {
        int32_t index = dic.findEntry(parent, code);
        if (index<0) { // entry not found
            writeCode(out, &buffer, &pos, &bitsUsed, bitsPerCode, parent);
            if (dic.index>4092) {
                writeCode(out,  &buffer, &pos, &bitsUsed, bitsPerCode, LZW_RESET_CODE);
                dic.reset();
                bitsPerCode = 9;
            } else {
                dic.addEntry(parent, code, index);
                if (dic.index>=(1<<bitsPerCode))
                bitsPerCode++;
            }
            parent = code;
        }
        else
            parent = index;
    }
    if (parent>=0)
        writeCode(out, &buffer, &pos, &bitsUsed, bitsPerCode, parent);
    writeCode(out, &buffer, &pos, &bitsUsed, bitsPerCode, LZW_EOF_CODE);
    if (bitsUsed>0) { // flush buffer
        pos++;
        out->putc(uint8_t(buffer));
    }
    fsize=pos;
    return fsize;
}

lzwFilter::~lzwFilter() {
}

