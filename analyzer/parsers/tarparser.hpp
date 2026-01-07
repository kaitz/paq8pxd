#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include <cstring>

struct TARheader{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char linkflag;
    char linkname[100];
    char magic[8];
    char uname[32];
    char gname[32];
    char major[8];
    char minor[8];
    char pad[167];
};

class TARParser: public Parser {
    uint64_t tar,tarn,tarl,utar;
    TARheader tarh;
    uint8_t tars[512];
    uint32_t tarsi;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    uint32_t flCount;
    int getoct(const char *p, int n);
    int tarchecksum(char *p);
    bool tarend(const char *p);
public:    
    TARParser();
    ~TARParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
