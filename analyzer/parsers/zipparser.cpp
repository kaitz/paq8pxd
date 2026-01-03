#include "zipparser.hpp"
#include <cstdio>

ZIPParser::ZIPParser() {
    priority = 1;
    Reset();
    name = "zip";
}

ZIPParser::~ZIPParser() {
}

// Detect ZIP local file headers (PK\x03\x04) with deflate compression
DetectState ZIPParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // Too small?
    if (pos == 0 && len < 32) return DISABLE;
    
    // Are we in new data block, if so reset inSize and restart
    if (inpos != pos) {
        inSize = 0;
        inpos = pos;
        i = pos;
    }
    
    while (inSize < len) {
        buf3 = (buf3 << 8) | (buf2 >> 24);
        buf2 = (buf2 << 8) | (buf1 >> 24);
        buf1 = (buf1 << 8) | (buf0 >> 24);
        uint8_t c = data[inSize];
        buf0 = (buf0 << 8) + c;
        
        if (state == NONE) {
            // ZIP local file header signature in little-endian: 0x04034b50
            // But we're reading big-endian into buf0, so check for 0x504B0304
            if (buf0 == 0x504B0304) {
                // We need 30 bytes from signature start to read header
                // Current position: inSize (signature ends here, byte 0x04 just read)
                // Signature starts at: inSize - 3
                // We need bytes at offsets 8-9, 18-21, 26-29 from signature start
                // So we need: sig_start + 29 < len, i.e., inSize - 3 + 29 < len, i.e., inSize + 26 < len
                
                if (inSize >= 3 && inSize + 26 < len) {
                    int sig_start = inSize - 3;
                    
                    // Compression method at offset 8 (little-endian)
                    int compression_method = data[sig_start + 8] | (data[sig_start + 9] << 8);
                    
                    if (compression_method == 8) {  // Deflate
                        // Compressed size at offset 18 (little-endian)
                        uint32_t compressed_size = 
                            data[sig_start + 18] |
                            (data[sig_start + 19] << 8) |
                            (data[sig_start + 20] << 16) |
                            (data[sig_start + 21] << 24);
                        
                        // Filename and extra field lengths at offset 26-29
                        uint16_t filename_len = data[sig_start + 26] | (data[sig_start + 27] << 8);
                        uint16_t extra_len = data[sig_start + 28] | (data[sig_start + 29] << 8);
                        
                        // Header length: 30 + filename + extra
                        int header_len = 30 + filename_len + extra_len;
                        
                        // Sanity checks
                        if (compressed_size > 0 && compressed_size < 0x7FFFFFFF &&
                            filename_len < 1024 && extra_len < 65535) {
                            
                            // Stream starts after header (global position)
                            jstart = (i - 3) + header_len;
                            jend = jstart + compressed_size;
                            type = ZIP;
                            info = 0;  // recursive
                            state = END;

                            return END;
                        }
                    }
                }
            }
        }
        
        inSize++;
        i++;
    }
    
    return NONE;
}

dType ZIPParser::getType(int idx) {
    dType t;
    t.start = jstart;
    t.end = jend;
    t.info = info;
    t.rpos = 0;
    t.type = type;
    t.recursive = false;  // false for sequential entries; TR_RECURSIVE in streams handles content recursion
    return t;
}

int ZIPParser::TypeCount() {
    return 1;
}

void ZIPParser::Reset() {
    state = NONE;
    type = DEFAULT;
    jstart = jend = buf0 = buf1 = buf2 = buf3 = 0;
    info = i = inSize = inpos = 0;
}

void ZIPParser::SetEnd(uint64_t e) {
    jend = e;
}
