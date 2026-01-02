#include "gzipparser.hpp"
#include <cstdio>
#include <cstring>

GZIPParser::GZIPParser() {
    priority = 1;
    strm = new z_stream;
    Reset();
    name = "gzip";
}

GZIPParser::~GZIPParser() {
    delete strm;
}

// Detect GZip headers: \x1f\x8b\x08 (magic + deflate method)
// Simple approach: detect header and return END with jend = UINT64_MAX - 8
// The Analyzer clips jend to file_size, which works for whole-file gzip.
// The preflateFilter handles actual deflate stream length during decode.
// NOTE: This approach fails for small gzip files embedded in larger containers
// (e.g., gzip inside tar). Proper support would require File* access from parser.

DetectState GZIPParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    if (pos == 0 && len < 18) return DISABLE;
    
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
            // GZip header: [0x1f][0x8b][0x08][flags]
            if ((buf0 >> 8) == 0x1F8B08) {
                uint8_t flags = buf0 & 0xFF;
                
                // Check reserved bits (bits 5-7 must be 0)
                if ((flags & 0xE0) == 0) {
                    uint64_t hdr_start = i - 3;
                    int hdr_pos = inSize - 3;
                    
                    if (hdr_pos < 0 || hdr_pos + 10 > (int)len) {
                        inSize++; i++; continue;
                    }
                    
                    // Calculate header length
                    int header_len = 10;
                    int scan_pos = hdr_pos + 10;
                    
                    // FEXTRA
                    if (flags & 0x04) {
                        if (scan_pos + 2 > (int)len) { inSize++; i++; continue; }
                        int xlen = data[scan_pos] | (data[scan_pos + 1] << 8);
                        header_len += 2 + xlen;
                        scan_pos += 2 + xlen;
                    }
                    
                    // FNAME - null-terminated original filename
                    if (flags & 0x08) {
                        while (scan_pos < (int)len && data[scan_pos] != 0) {
                            scan_pos++; header_len++;
                        }
                        if (scan_pos >= (int)len) { inSize++; i++; continue; }
                        scan_pos++; header_len++;
                    }
                    
                    // FCOMMENT
                    if (flags & 0x10) {
                        while (scan_pos < (int)len && data[scan_pos] != 0) {
                            scan_pos++; header_len++;
                        }
                        if (scan_pos >= (int)len) { inSize++; i++; continue; }
                        scan_pos++; header_len++;
                    }
                    
                    // FHCRC
                    if (flags & 0x02) {
                        header_len += 2;
                    }
                    
                    // Deflate stream starts after header
                    jstart = hdr_start + header_len;
                    
                    // Set jend to a very large value minus 8 (for CRC32+ISIZE trailer)
                    // Analyzer will clip this to file_size - 8
                    jend = UINT64_MAX - 8;
                    
                    type = GZIP;
                    info = 0;
                    state = END;
                    return END;
                }
            }
        }
        
        inSize++;
        i++;
    }
    
    return NONE;
}

dType GZIPParser::getType(int idx) {
    dType t;
    t.start = jstart;
    t.end = jend;
    t.info = info;
    t.rpos = 0;
    t.type = type;
    t.recursive = false;
    return t;
}

int GZIPParser::TypeCount() {
    return 1;
}

void GZIPParser::Reset() {
    state = NONE;
    type = DEFAULT;
    jstart = jend = buf0 = buf1 = buf2 = buf3 = 0;
    info = i = inSize = inpos = expected_next_pos = 0;
    memset(zout, 0, sizeof(zout));
}

void GZIPParser::SetEnd(uint64_t e) {
    jend = e;
}
