#pragma once
#include "types.hpp"
#include "array.hpp"
#include <stdio.h>

// Error handler: print message if any, and exit
void quit(const char* message) ;

// strings are equal ignoring case?
int equals(const char* a, const char* b);

bool makedir(const char* dir);
void makedirectories(const char* filename);
FILE* tmpfile2(void);

U32 utf8_check(U8 *s);
U64 MEM();
    U64 CMlimit(U64 size);
