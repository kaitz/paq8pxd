#pragma once
#include "parser.hpp"
#include "../prt/enums.hpp"
#include "../prt/file.hpp"
#include <cstdint>
#include <vector>
#include "parsers/bmpparser.hpp"
#include "parsers/default.hpp"

class Analyser {
    uint64_t info;
    std::vector<Parser*> parsers;
    void AddParser(Parser *p);
    uint64_t remaining;
    dType def;
    dType found;
    bool typefound;
public:    
    Analyser();
    ~Analyser();
    dType Detect(File* in, U64 n, int it=0);
};
