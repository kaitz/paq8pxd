#pragma once
#include "parser.hpp"
#include "../prt/enums.hpp"
#include "../prt/file.hpp"
#include <cstdint>
#include <vector>
#include "parsers/bmpparser.hpp"
#include "parsers/default.hpp"
#include "parsers/textparser.hpp"

class Analyser {
    uint64_t info;
    std::vector<Parser*> parsers;
    void AddParser(Parser *p);
    uint64_t remaining;
    std::vector<dType> types;
    size_t lastType;
    dType currentType;
    dType emptyType;
    bool typefound;
public:    
    Analyser();
    ~Analyser();
    bool Detect(File* in, U64 n, int it=0);
    dType GetNext();
};
