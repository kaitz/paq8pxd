#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include <cstring>
// image in pdf
//  'BI
//   /W 86
//   /H 85
//   /BPC 1 
//   /IM true
//   ID '
   
class pdfBiParser: public Parser {
    uint64_t pdfi1,pdfiw,pdfih,pdfic;
    char pdfi_buf[32];
    int pdfi_ptr,pdfin;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    pdfBiParser();
    ~pdfBiParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
