#include "zipparser.hpp"
#include <cstdio>

ZIPParser::ZIPParser() {
    priority = 2;
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
                    int comp_flags = data[sig_start + 8-2] | (data[sig_start + 9-2] << 8);
                    // Compression method at offset 8 (little-endian)
                    int comp_method = data[sig_start + 8] | (data[sig_start + 9] << 8);
                    /*
                    00 no compression
                    01 shrunk
                    02 reduced with compression factor 1
                    03 reduced with compression factor 2
                    04 reduced with compression factor 3
                    05 reduced with compression factor 4
                    06 imploded
                    07 reserved
                    08 deflated
                    09 enhanced deflated
                    10 PKWare DCL imploded
                    11 reserved
                    12 compressed using BZIP2
                    13 reserved
                    14 LZMA
                    15-17: reserved
                    // ignore below
                    18 compressed using IBM TERSE
                    19 IBM LZ77 z
                    98 PPMd version I, Rev 1
                    */
                    if (comp_method<15) {
                        // Compressed size at offset 18 (little-endian)
                        uint32_t compressed_size = 
                            data[sig_start + 18] |
                            (data[sig_start + 19] << 8) |
                            (data[sig_start + 20] << 16) |
                            (data[sig_start + 21] << 24);
                        uint32_t uncompressed_size = 
                            data[sig_start + 18+4] |
                            (data[sig_start + 19+4] << 8) |
                            (data[sig_start + 20+4] << 16) |
                            (data[sig_start + 21+4] << 24);
                        
                        // Filename and extra field lengths at offset 26-29
                        filename_len = data[sig_start + 26] | (data[sig_start + 27] << 8);
                        uint16_t extra_len = data[sig_start + 28] | (data[sig_start + 29] << 8);
                        
                        // Header length: 30 + filename + extra
                        int header_len = 30 + filename_len + extra_len;
                        fpos=inSize;
                        // Sanity checks
                        if (comp_method==7 || comp_method==11 || comp_method==13 || comp_method>14 || filename_len>255) {
                            state=NONE;
                        } else if (compressed_size > 0 && compressed_size < 0x7FFFFFFF &&
                            filename_len < 1024 && extra_len < 65535) {
                            // Stream starts after header (global position)
                            jstart = (i - 3) + header_len;
                            jend = jstart + compressed_size;
                            type = comp_method==8?ZIP:CMP;
                            type = comp_method==0?RECE:type;
                            type = comp_method==1?SHRINK:type;
                            //type = comp_method==5?REDUCE:type; // needs work or reconstruction info
                            //type = comp_method==6?IMPLODE:type;// needs work or reconstruction info
                            //printf("Flags %d\n",comp_flags);
                            if (type==SHRINK || type==REDUCE || type==IMPLODE) info=uint64_t(uncompressed_size)<<32;
                            else info = 0;
                            state = INFO;
                            fname="";
                        }
                    }
                }
            }
        } else if (state==INFO && i>fpos && filename_len) {
            for (int j=0; j<filename_len; j++) fname+=data[(fpos-5+0x20+j)&0xffff];
            //printf("%s\n",fname.c_str());
            info|=GetTypeFromExt(fname);
            state=END;
            return state;
        }
        
        inSize++;
        i++;
    }
    
    return NONE;
}

dType ZIPParser::getType() {
    dType t;
    t.start = jstart;
    t.end = jend;
    t.info = info;
    t.rpos = 0;
    t.type = type;
    t.recursive = false;  // false for sequential entries; TR_RECURSIVE in streams handles content recursion
    return t;
}

void ZIPParser::Reset() {
    state = NONE;
    type = DEFAULT;
    jstart = jend = buf0 = buf1 = buf2 = buf3 = 0;
    info = i = inSize = inpos = 0;
    filename_len=0;
    priority = 2;
    fname="";
}
