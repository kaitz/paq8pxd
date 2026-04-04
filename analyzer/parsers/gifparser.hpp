#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class GIFParser: public Parser {
    uint64_t gif, gifa, gifi, gifw, gifc, gifb, plt, gray;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type,subtype;
    uint64_t jstart, jend, inSize, inpos;
    bool rec;
    int TCOLORS;
    std::vector<ColorRGBA> bmcolor;
    uint64_t pals;
public:
    uint8_t palo[256];
    GIFParser();
    ~GIFParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    void ReorderPal(unsigned char *data);
    dType getType() final;
    void Reset() final;
};
