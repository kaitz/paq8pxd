#pragma once
#include "array.hpp"
#include <stdio.h>

// Error handler: print message if any, and exit
void quit(const char* message=0) ;

// strings are equal ignoring case?
int equals(const char* a, const char* b);

bool makedir(const char* dir);
void makedirectories(const char* filename);
FILE* tmpfile2(void);



