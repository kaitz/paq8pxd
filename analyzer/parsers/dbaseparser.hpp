#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include <mem.h>

// dBASE VERSIONS
//  '02' > FoxBase
//  '03' > dBase III without memo file
//  '04' > dBase IV without memo file
//  '05' > dBase V without memo file
//  '07' > Visual Objects 1.x
//  '30' > Visual FoxPro
//  '31' > Visual FoxPro with AutoIncrement field
//  '43' > dBASE IV SQL table files, no memo
//  '63' > dBASE IV SQL system files, no memo
//  '7b' > dBase IV with memo file
//  '83' > dBase III with memo file
//  '87' > Visual Objects 1.x with memo file
//  '8b' > dBase IV with memo file
///  '8e' > dBase IV with SQL table
//  'cb' > dBASE IV SQL table files, with memo
//  'f5' > FoxPro with memo file - tested
//  'fb' > FoxPro without memo file

class dBaseParser: public Parser {
struct dBASE {
  uint8_t Version;
  uint32_t nRecords;
  uint16_t RecordLength, HeaderLength;
  int Start, End;
};
    dBASE dbase;
    uint64_t dbasei,term;
  
    uint64_t info;
    uint32_t buf0, buf1;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    dBaseParser();
    ~dBaseParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};


