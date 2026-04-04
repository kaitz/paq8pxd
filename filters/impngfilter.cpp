#include "impngfilter.hpp"

ImPngFilter::ImPngFilter(std::string n, Settings &s, Filetype f):Filter(s) {  
    name=n;
    Type=f;
}

uint8_t ImPngFilter::paeth(uint8_t const W, uint8_t const N, uint8_t const NW) {
    int p=W+N-NW;
    int pW=abs(p-W);
    int pN=abs(p-N);
    int pNW=abs(p-NW);
    if (pW<=pN && pW<=pNW) {
        return W;
    }
    if (pN<=pNW) {
        return N;
    }
    return NW;
}

void ImPngFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    Filetype type2 =(Filetype)(info>>24);
    int stride = type2 == IMAGE24 ? 3 : type2 == IMAGE32 ? 4 : 1;
    int width=int(info&0xffffff);
    int lineWidth = width + 1; //including filter byte
    int headerSize = static_cast<int>(size / lineWidth); // = number of rows
    RingBuffer filterBuffer(nextPowerOf2(headerSize));
    RingBuffer pixelBuffer(nextPowerOf2(size /*- headerSize*/));
    out->put32(headerSize);
    hdrsize=headerSize;
    //assert(filterBuffer.size() >= headerSize);
    //assert(pixelBuffer.size() >= size - headerSize);
    for (int line = 0; line < headerSize; line++ ) {
        uint8_t filter = in->getc();
        filterBuffer.Add(filter);
        for (int x = 0; x < width; x++) {
            uint8_t c1 = in->getc();
            switch (filter) {
            case 0: {
                    break;
                }
            case 1: {
                    c1=(static_cast<uint8_t>(c1 + (x < stride ? 0 : pixelBuffer(stride))));
                    break;
                }
            case 2: {
                    c1=(static_cast<uint8_t>(c1 + (line == 0 ? 0 : pixelBuffer(width))));
                    break;
                }
            case 3: {
                    c1 = (static_cast<uint8_t>(c1 + (((line == 0 ? 0 : pixelBuffer(width)) + (x < stride ? 0 : pixelBuffer(stride))) >> 1)));
                    break;
                }
            case 4: {
                    c1 = (static_cast<uint8_t>(c1 + paeth(
                    x < stride ? 0 : pixelBuffer(stride),
                    line == 0 ? 0 : pixelBuffer(width),
                    line == 0 || x < stride ? 0 : pixelBuffer(width + stride))));
                    break;
                }
            default:
                //fail: unexpected filter code.
                return;
            }
            pixelBuffer.Add(c1);
        }
    }
    uint32_t len1=filterBuffer.getpos();
    uint32_t len2=pixelBuffer.getpos();
    for (uint32_t i=0; i<len1; i++)
    out->putc(filterBuffer[i]);
    for (uint32_t i=0; i<len2; i++)
    out->putc(pixelBuffer[i]);
} 

uint64_t ImPngFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    Filetype type2 =(Filetype)(info>>24);
    int stride = type2 == IMAGE24 ? 3 : type2 == IMAGE32 ? 4 : 1;
    int width=int(info&0xffffff);
    int lineWidth = width + 1; //including filter byte
    int headerSize = static_cast<int>(size / lineWidth); // = number of rows
    headerSize=in->get32();
    RingBuffer filterBuffer(nextPowerOf2(headerSize));
    RingBuffer pixelBuffer(nextPowerOf2(size /*- headerSize*/));
    //assert(filterBuffer.size() >= headerSize);
    //assert(pixelBuffer.size() >= size - headerSize);
    for (int line = 0; line < headerSize; line++) {
        uint8_t filter = in->getc();
        filterBuffer.Add(filter);
    }
    uint32_t p = 0;
    for (int line = 0; line < headerSize; line++) {
        uint8_t filter = filterBuffer[line];
        out->putc(filter);
        for (int x = 0; x < width; x++) {
            uint8_t c1 = in->getc();
            uint8_t c = c1;
            switch (filter) {
            case 0: {
                    break;
                }
            case 1: {
                    c1 = (static_cast<uint8_t>(c1 - (x < stride ? 0 : pixelBuffer(stride))));
                    break;
                }
            case 2: {
                    c1 = (static_cast<uint8_t>(c1 - (line == 0 ? 0 : pixelBuffer(width))));
                    break;
                }
            case 3: {
                    c1 = (static_cast<uint8_t>(c1 - (((line == 0 ? 0 : pixelBuffer(width)) + (x < stride ? 0 : pixelBuffer(stride))) >> 1)));
                    break;
                }
            case 4: {
                    c1 = (static_cast<uint8_t>(c1 - paeth(
                    x < stride ? 0 : pixelBuffer(stride),
                    line == 0 ? 0 : pixelBuffer(width),
                    line == 0 || x < stride ? 0 : pixelBuffer(width + stride))));
                    break;
                }
            default:
                //fail: unexpected filter code.
                break;
            }
            pixelBuffer.Add(c);
            out->putc(c1);
        }
    }
    fsize=size=out->curpos();
    return size;
}

ImPngFilter::~ImPngFilter() {
}

