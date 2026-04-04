#include "preflatefilter.hpp"
#include "preflate_adapters.hpp"
#include "../preflate/preflate.h"
#include <vector>

preflateFilter::preflateFilter(std::string n, Settings &s, Filetype f):Filter(s) {
    name = n;
    Type = f;
}

preflateFilter::~preflateFilter() {}

void preflateFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    // Look for header/footer data
    uint32_t hdr=info>>16;
    uint32_t footr=info&0xffff;
    std::vector<uint8_t> deflate_raw(size-hdr-footr);
    if (hdr) for (int i=0; i<hdr; i++) out->putc(in->getc());
    // Read entire deflate stream into vector
    in->blockread(deflate_raw.data(), size-hdr-footr);
    if (footr) for (int i=0; i<footr; i++) out->putc(in->getc());
    hdrsize=hdr+footr;
    // Decode with preflate: deflate -> unpacked + reconstruction info
    std::vector<uint8_t> unpacked;
    std::vector<uint8_t> recon_info;
    
    if (!preflate_decode(unpacked, recon_info, deflate_raw)) {
        out->put32(static_cast<uint32_t>(-1)); // set -1 for decoder
        diffFound = 1;
        return;
    }
    
    // Write format:
    // [4 bytes: recon_info size]
    // [recon_info bytes]
    // [unpacked data] 
    
    out->put32(static_cast<uint32_t>(recon_info.size()));
    hdrsize+=static_cast<uint32_t>(recon_info.size())+4;
    out->blockwrite(recon_info.data(), recon_info.size());
    out->blockwrite(unpacked.data(), unpacked.size());
    
    diffFound = 0;
}

uint64_t preflateFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    // Look for header/footer data
    uint32_t hdr=info>>16;
    uint32_t footr=info&0xffff;
    if (hdr) for (int i=0; i<hdr; i++) out->putc(in->getc());
    Array<uint8_t> ftr(footr);
    if (footr) for (int i=0; i<footr; i++) ftr[i]=(in->getc());
    hdrsize=hdr+footr;
    // Read reconstruction info size
    uint32_t recon_size = in->get32();
    if (recon_size==0xffffffff) return 0; // fail if encoder set -1
    // Read reconstruction info
    std::vector<uint8_t> recon_info(recon_size);
    in->blockread(recon_info.data(), recon_size);
    
    // Read unpacked data (remaining bytes)
    uint64_t unpacked_size = size - 4 - recon_size-hdrsize;
    std::vector<uint8_t> unpacked(unpacked_size);
    in->blockread(unpacked.data(), unpacked_size);
    
    // Reencode: unpacked + recon_info -> original deflate
    std::vector<uint8_t> deflate_raw;
    
    if (!preflate_reencode(deflate_raw, recon_info, unpacked)) {
        fsize = 0;
        return 0;
    }
    
    // Write reconstructed deflate stream
    out->blockwrite(deflate_raw.data(), deflate_raw.size());
    if (footr) for (int i=0;i<footr;i++) out->putc(ftr[i]);
    fsize = deflate_raw.size()+hdrsize;
    hdrsize+=recon_size+4;
    return fsize;
}
