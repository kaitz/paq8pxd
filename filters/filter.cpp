#include "filter.hpp"

Filter::Filter():HeaderLength(0),DataStart(0),DataLength(0),Type(DEFAULT),Info(0),state(NONE),name(""),diffFound(0) {
}

Filter::~Filter(){}

FMode Filter::compare(File *in, File *out, uint64_t size) {
    diffFound=0;
    for (uint64_t j=0; j<size; ++j) {
        int a=in->getc();
        int b=out->getc();
        if (a!=b && !diffFound) {
            diffFound=j+1;
            return FDISCARD;
        }
    }
    return FEQUAL;
}
