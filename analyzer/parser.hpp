#pragma once
#include "../prt/enums.hpp"
#include <cstdint>
#include <vector>
// base class for file type detection

struct dType {
    uint64_t start;     // start pos of type data in block
    uint64_t end;       // end pos of type data in block
    uint64_t info;      // info of the block if present
    uint64_t rpos;      // pos where start was set in block
    Filetype type;
    bool     recursive; // is data recursive
};

class Parser {
public:
    DetectState state;       // State of current detection
    std::vector<dType> type; // For multiple data in detected format.
                             // Some formats (tar, tiff, etc..) have 
                             // multiple (non)recursive data types inside. We collect data for all
                             // until main file ends. At the same time report back last found type.
    int priority;            // From 0-4, where 0 top level (recursive) and 4 is bottom (least important)
                             // 0 - container format (tar, mdf, ...)
                             // 1 - recursive (zlib, bzip2, base64, ...)
                             // 2 - single or multi data (bmp, tif, ...)
                             // 3 - everything else (text, ...)
                             // 4 - default
    std::string name;
    Parser();
    ~Parser();
    virtual DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos)=0;
    virtual dType getType(int i)=0;
    virtual int TypeCount()=0;
    virtual void Reset()=0;
};
