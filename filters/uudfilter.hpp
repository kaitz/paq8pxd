#pragma once
#include "filter.hpp"

#define UUENCODE(c,b) ((c) ? ((c) & 077) + ' ': (b) ? '`':((c) & 077) + ' ')
#define UUDECODE(c) (((c) - ' ') & 077)

class uudFilter: public Filter {
public:
    uudFilter(std::string n, Filetype f=DEFAULT);
    ~uudFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};

