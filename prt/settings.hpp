#pragma once
#include "enums.hpp"
#include <cstdint>
#include <string>

#ifndef DEFAULT_OPTION
#define DEFAULT_OPTION 8
#endif

struct Settings {
    uint8_t level;
    bool slow;
    bool fast;
    bool witmode;
    bool staticd;
    bool doExtract;
    bool doList;
    bool showhelp;
    bool isConRedirected;
    int verbose;
    int rdepth;
    std::string externaDict;
    int minfq;
    int topt;
    ParserType userPT[P_LAST]={P_DEF}; // User defined parser list
    int userPTsize;
    Settings();
};
