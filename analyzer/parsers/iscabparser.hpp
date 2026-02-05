#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include <set>

// For IS CAB detection (ver 5)
// File contains only zlib streams.
// File name and other info in the .hdr file.
// We have no info about how many blocks there are,
// so assume that we parse whole cab file.

struct CABHeader {
    uint32_t Signature;
    uint32_t Version;
    uint8_t NextVol;
    uint8_t junk2;
    uint16_t junk3;
    uint32_t ofsCabDesc;
    uint32_t cbCabDesc;
    uint32_t ofsCompData;
    uint32_t junk1;
    uint32_t FirstFile;
    uint32_t LastFile;
    uint32_t ofsFirstData;
    uint32_t cbFirstExpanded;
    uint32_t cbFirstHere;
    uint32_t ofsLastData;
    uint32_t cbLastExpanded;
    uint32_t cbLastHere;
};

struct ISCfile {
    uint64_t start;
    uint64_t size;
    ParserType p;
};

class ISCABParser: public Parser { 
    uint64_t isc;
    uint32_t spos;
    uint32_t scount;
    Array<ISCfile> iscF;
    uint64_t iscFiles;
    bool rec;
    uint64_t relAdd;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    ISCABParser();
    ~ISCABParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
