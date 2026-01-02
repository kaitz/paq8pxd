#pragma once
#include "parser.hpp"
#include "../prt/enums.hpp"
#include "../prt/file.hpp"
#include <cstdint>
#include <vector>
#include "parsers/bmpparser.hpp"
#include "parsers/default.hpp"
#include "parsers/textparser.hpp"
#include "parsers/decaparser.hpp"
#include "parsers/mrbparser.hpp"
#include "parsers/exeparser.hpp"
#include "parsers/zlibparser.hpp"
#include "parsers/nesparser.hpp"
#include "parsers/mszipparser.hpp"
#include "parsers/jpegparser.hpp"
#include "parsers/wavparser.hpp"
#include "parsers/pnmparser.hpp"
#include "parsers/pdflzwparser.hpp"
#include "parsers/gifparser.hpp"
#include "parsers/dbaseparser.hpp"
#include "parsers/pdfbiparser.hpp"
#include "parsers/aiffparser.hpp"
#include "parsers/ascii85parser.hpp"
#include "parsers/base641parser.hpp"
#include "parsers/base642parser.hpp"
#include "parsers/modparser.hpp"
#include "parsers/sgiparser.hpp"
#include "parsers/tgaparser.hpp"
#include "parsers/cdparser.hpp"
#include "parsers/mdfparser.hpp"
#include "parsers/uueparser.hpp"
#include "parsers/tiffparser.hpp"
#include "parsers/tarparser.hpp"
#include "parsers/pngparser.hpp"
#include "parsers/bzip2parser.hpp"
#include "parsers/szddparser.hpp"
#include "parsers/mscfparser.hpp"
#include "parsers/zipparser.hpp"

class Analyzer {
    uint64_t info;
    std::vector<Parser*> parsers;
    void AddParser(Parser *p);
    uint64_t remaining;
    std::vector<dType> types;
    size_t lastType;
    dType currentType;
    dType emptyType;
    bool typefound;
    int iter;
    Filetype ptype;
    std::string *pinfo;
    void Status(uint64_t n, uint64_t size);
public:    
    Analyzer(int it,Filetype p=DEFAULT);
    ~Analyzer();
    bool Detect(File* in, U64 n, int it=0);
    dType GetNext();
    std::string &GetInfo();
};
