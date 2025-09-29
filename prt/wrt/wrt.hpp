#pragma once
#define PRINT_CHARS(data) ;//printf data
#define PRINT_CODEWORDS(data); // printf data
#define PRINT_STACK(data) ;//printf data;
#define PRINT_DICT(data) ;//printf data;
#define PRINT_CONTAINERS(data) ;//printf data
//#define PRINT_STATUS(data) printf data;


#define CHAR_FIRSTUPPER     1   // for encode lower word with first capital letter
#define CHAR_UPPERWORD      2   // for encode upper word
#define CHAR_ESCAPE         3   // for encode reserved chars (CHAR_ESCAPE,CHAR_FIRSTUPPER,...)
#define CHAR_UTFUPPER       4
#define CHAR_EOL            5   // for enocde linefeed in EOLencoder not in wrt
#define BINARY_FIRST        128
#define BINARY_LAST         255


#define WORD_AVG_SIZE       8
#define WORD_MAX_SIZE       48
#define STRING_MAX_SIZE     255  // 1-byte for container.size()


#define HASH_TABLE_SIZE         (1<<20) //1MB*4
#define HASH_DOUBLE_MULT    37
#define HASH_MULT           23

enum ELetterType { LOWERCHAR, UPPERCHAR, UNKNOWNCHAR, RESERVEDCHAR, NUMBERCHAR };
