#include "settings.hpp"

Settings::Settings(): 
    level(DEFAULT_OPTION),  // Compression level 0 to 15
    slow(false),            // -x option
    fast(false),            // -f option
    witmode(false),         // -w option
    staticd(false),         // -e option
    doExtract(false),       // -d option
    doList(false),          // -l option
    showhelp(false),        // -h option
    isConRedirected(false), // Is console redirected by >
    verbose(0),             // -v option
    rdepth(6),              // -r option
    externaDict(""),        // -e option 
    minfq(19),              // global word frq 
    topt(1),                // num of threads
    userPTsize(1)           // user defined parser count,  1= no parsers 
{
};
