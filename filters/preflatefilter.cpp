#include "preflatefilter.hpp"
#include "preflate_adapters.hpp"
#include "preflate/preflate.h"
#include <vector>

preflateFilter::preflateFilter(std::string n, Filetype f) {
    name = n;
    Type = f;
}

preflateFilter::~preflateFilter() {}

void preflateFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    // Read entire deflate stream into vector
    std::vector<uint8_t> deflate_raw(size);
    in->blockread(deflate_raw.data(), size);
    
    // Decode with preflate: deflate -> unpacked + reconstruction info
    std::vector<uint8_t> unpacked;
    std::vector<uint8_t> recon_info;
    
    if (!preflate_decode(unpacked, recon_info, deflate_raw)) {
        diffFound = 1;
        return;
    }
    
    // Write format:
    // [4 bytes: recon_info size]
    // [recon_info bytes]
    // [unpacked data]
    
    out->put32(static_cast<uint32_t>(recon_info.size()));
    out->blockwrite(recon_info.data(), recon_info.size());
    out->blockwrite(unpacked.data(), unpacked.size());
    
    diffFound = 0;
}

uint64_t preflateFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    // Read reconstruction info size
    uint32_t recon_size = in->get32();
    
    // Read reconstruction info
    std::vector<uint8_t> recon_info(recon_size);
    in->blockread(recon_info.data(), recon_size);
    
    // Read unpacked data (remaining bytes)
    uint64_t unpacked_size = size - 4 - recon_size;
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
    
    fsize = deflate_raw.size();
    return fsize;
}
