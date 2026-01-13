#pragma once
#include "../parser.hpp"
#include "../../zlib/zlib.h"
#include <cstdint>

class GZIPParser: public Parser {
    uint32_t buf0, buf1, buf2, buf3;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    uint64_t info;
    uint8_t zout[1<<16];  // Inflation output buffer
    z_stream *strm;
    int hdr_pos_in_block;  // Position of gzip header start in current block
    uint64_t expected_next_pos;  // Expected position for next block when in INFO state
public:
    GZIPParser();
    ~GZIPParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
