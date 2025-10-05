#include "rlefilter.hpp"

rleFilter::rleFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
} 

void rleFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    U8 b, c=in->getc();
    int i=1, maxBlockSize=info&0xFFFFFF;
    out->put32(maxBlockSize);
    //hdrsize=(4);
    while (i<(int)size) {
        b=in->getc(), i++;
        if (c==0x80) { c=b; continue; }
        else if (c>0x7F) {
            for (int j=0; j<=(c&0x7F); j++) out->putc(b);
            c=in->getc(), i++;
        }
        else {
            for (int j=0; j<=c; j++, i++) { out->putc(b), b=in->getc(); }
            c=b;
        }
    }
} 

uint64_t rleFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    U8 inBuffer[0x10000]={0};
    U8 outBuffer[0x10200]={0};
    U64 pos = 0;
    int maxBlockSize=(int)in->get32();
    enum { BASE, LITERAL, RUN, LITERAL_RUN } state;
    do {
        U64 remaining = in->blockread(&inBuffer[0], maxBlockSize);
        U8 *inPtr = (U8*)inBuffer;
        U8 *outPtr= (U8*)outBuffer;
        U8 *lastLiteral = nullptr;
        state = BASE;
        while (remaining>0){
            U8 byte = *inPtr++, loop = 0;
            int run = 1;
            for (remaining--; remaining>0 && byte==*inPtr; remaining--, run++, inPtr++);
            do {
                loop = 0;
                switch (state) {
                case BASE: case RUN: {
                        if (run>1) {
                            state = RUN;
                            rleOutputRun;
                        }
                        else {
                            lastLiteral = outPtr;
                            *outPtr++ = 0, *outPtr++ = byte;
                            state = LITERAL;
                        }
                        break;
                    }
                case LITERAL: {
                        if (run>1) {
                            state = LITERAL_RUN;
                            rleOutputRun;
                        }
                        else {
                            if (++(*lastLiteral)==127)
                            state = BASE;
                            *outPtr++ = byte;
                        }
                        break;
                    }
                case LITERAL_RUN: {
                        if (outPtr[-2]==0x81 && *lastLiteral<(125)) {
                            state = (((*lastLiteral)+=2)==127)?BASE:LITERAL;
                            outPtr[-2] = outPtr[-1];
                        }
                        else
                        state = RUN;
                        loop = 1;
                    }
                }
            } while (loop);
        }

        U64 length = outPtr-(U8*)(&outBuffer[0]);
        out->blockwrite(&outBuffer[0], length);
        pos+=length;
    } while (!in->eof());
    fsize=pos;
    return fsize;
}

rleFilter::~rleFilter() {
}

