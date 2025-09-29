#pragma once
#include "types.hpp"
#include "array.hpp"
#include "log.hpp"
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
#define bswap64(n) ((bswap((n&0xFFFFFFFF00000000)>>32))|((bswap(n&0x00000000FFFFFFFF))<<32))
U8 Clip(int const Px);

#define TAB 0x09
#define NEW_LINE 0x0A
#define CARRIAGE_RETURN 0x0D
#define SPACE 0x20
#define QUOTE 0x22
#define APOSTROPHE 0x27

 int min(int a, int b);
 int max(int a, int b);

U8 Clamp4(const int Px, const U8 n1, const U8 n2, const U8 n3, const U8 n4);
  U8 LogMeanDiffQt(const U8 a, const U8 b, const U8 limit = 7);
  U32 LogQt(const U8 Px, const U8 bits);

  U8 Paeth(U8 W, U8 N, U8 NW);
  #define MAX_WORD_SIZE 64
   bool CharInArray(const char c, const char a[], const int len) ;
