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
    
template<typename T>
constexpr auto isPowerOf2(T x) -> bool {
  return ((x & (x - 1)) == 0);
}

#define bswap(x) \
+   ((((x) & 0xff000000) >> 24) | \
+    (((x) & 0x00ff0000) >>  8) | \
+    (((x) & 0x0000ff00) <<  8) | \
+    (((x) & 0x000000ff) << 24))

U8 Clip(int const Px);

#define TAB 0x09
#define NEW_LINE 0x0A
#define CARRIAGE_RETURN 0x0D
#define SPACE 0x20

inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}
