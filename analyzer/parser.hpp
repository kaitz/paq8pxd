#pragma once
#include "../prt/enums.hpp"
#include <cstdint>
#include <vector>
#include <assert.h>

#define bswap(x) \
    +   ((((x) & 0xff000000) >> 24) | \
    +    (((x) & 0x00ff0000) >>  8) | \
    +    (((x) & 0x0000ff00) <<  8) | \
    +    (((x) & 0x000000ff) << 24))
    
#define bswap16(x) \
+    (((x) & 0xff00) >>  8) | \
+    (((x) & 0x00ff) <<  8)

#define MAX_PRI 7
// base class for file type detection

// Forward declaration - actual class in prt/file.hpp
class File;

struct dType {
    uint64_t start;     // start pos of type data in block
    uint64_t end;       // end pos of type data in block
    uint64_t info;      // info of the block if present
    uint64_t rpos;      // pos where start was set in block
    Filetype type;
    std::string pinfo;        // parser info string: width, etc
    bool     recursive; // is data recursive
};

class Parser {
public:
    DetectState state;       // State of current detection
    File* file_handle;       // Optional file handle for parsers that need to probe ahead
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
    std::string name;        // parser name
    std::string pinfo;        // parser info string: width, etc
    const std::string audiotypes[6]={"8b mono","8b stereo","16b mono","16b stereo","32b mono","32b stereo"};
    Parser();
    ~Parser();
    virtual DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last=false)=0;
    virtual dType getType(int i)=0;
    virtual int TypeCount()=0;
    virtual void Reset()=0;
    virtual void SetEnd(uint64_t e)=0;
    std::string itos(int64_t x, int n=1);
};
