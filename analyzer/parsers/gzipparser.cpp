#include "gzipparser.hpp"
#include "../../prt/file.hpp"
#include "../../filters/preflate_adapters.hpp"
#include "../../preflate/preflate.h"
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
// Uses file_handle to probe exact deflate stream length via preflate_decode

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
                    
                    // Use file_handle to probe exact stream length via preflate
                    if (file_handle != nullptr) {
                        // Save current file position
                        uint64_t saved_pos = file_handle->curpos();
                        
                        // Seek to deflate stream start
                        file_handle->setpos(jstart);
                        
                        // Use preflate_decode to find exact length
                        // Use a large estimate for available data (will stop at stream end)
                        uint64_t max_deflate = UINT64_MAX;  // Let preflate find the end
                        FileInputStream fis(file_handle, max_deflate);
                        
                        std::vector<unsigned char> unpacked;
                        std::vector<unsigned char> recon_info;
                        uint64_t deflate_size = 0;
                        
                        bool ok = preflate_decode(unpacked, recon_info, deflate_size, fis,
                                                  [](){}, 0);
                        
                        // Restore file position
                        file_handle->setpos(saved_pos);
                        
                        if (ok && deflate_size > 0) {
                            jend = jstart + deflate_size;
                            type = GZIP;
                            info = 0;
                            state = END;
                            return END;
                        }
                        // preflate failed (e.g., recursive stream with wrong file context)
                    }
                    
                    // Fall back to simple approach - set jend to max, Analyzer clips to file_size-8
                    jend = UINT64_MAX - 8;
                    type = GZIP;
                    info = 0;
                    state = END;
                    return END;
                }  // end if ((flags & 0xE0) == 0)
            }  // end if ((buf0 >> 8) == 0x1F8B08)
        }  // end if (state == NONE)
        
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
