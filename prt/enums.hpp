#pragma once
#include <string>
typedef enum {STR_NONE=-1,STR_DEFAULT=0,STR_JPEG, STR_IMAGE1, STR_IMAGE4, STR_IMAGE8, STR_IMAGE24, STR_AUDIO, STR_EXE, STR_TEXT0,STR_TEXT,STR_BIGTEXT,STR_DECA,STR_CMP,STR_LAST} Streamtype;
typedef enum {TR_NONE=0,TR_INFO=1,TR_RECURSIVE=2,TR_TRANSFORM=4, TR_FORWARD=8, TR_REVERSE=16,TR_DIRECT=32} Streamtypeinfo;

// Parser types
typedef enum {// real parsers
              P_DEF=0,P_BMP,P_TXT,P_DECA, P_MRB, P_EXE, P_NES, P_MZIP,P_JPG, P_WAV,
              P_PNM, P_PLZW, P_GIF, P_DBS, P_AIFF, P_A85, P_B641, P_B642,
              P_MOD, P_SGI, P_TGA, P_ICD, P_MDF, P_UUE, P_TIFF, P_TAR, P_PNG,
              P_ZIP, P_GZIP, P_BZIP2, P_SZDD, P_MSCF, P_ZLIB, P_ZLIBP,P_ISO9960,P_ISCAB,P_PBIT,
              P_LAST,
              // virtual parsers, see analyzer
              P_WEXE, P_WPDF, P_WCAB,
              P_WDEFAULT,
              P_WLAST
              } ParserType;              

typedef enum {DEFAULT=0,BINTEXT,ISOTEXT,DBASE, JPEG, HDR,CMP,IMGUNK, IMAGE1,IMAGE4, IMAGE8,IMAGE8GRAY, IMAGE24,IMAGE32, AUDIO, EXE,DECA,ARM,
              CD, TEXT,TEXT0, TXTUTF8,NESROM, BASE64, BASE85,UUENC, GIF ,SZDD,MRBR,MRBR4,RLE,LZW,BZIP2,
              ZLIB,PREFLATE,ZIP,GZIP,MDF,MSZIP,EOLTEXT,DICTTXT,BIGTEXT,NOWRT,TAR,PNG8, PNG8GRAY,IMPNG24,PNG24,IMPNG32, PNG32,RECE,WIT,
              ISO9960,ISCAB,SHRINK,REDUCE,IMPLODE,TYPELAST} Filetype;

static const int datatypecount=57;

static const char* typenames[datatypecount]={"default","bintext","ISO text","dBase", "jpeg", "hdr", "cmp","imgunk","1b-image", "4b-image", "8b-image","8b-gimage", "24b-image","32b-image", "audio",
                                "exe","DECa","ARM", "cd", "text","text0","utf-8","nes","base64","base85","uuenc","gif","SZDD","mrb","mrb4","rle","lzw","bzip2","zlib","preflate","zip","gzip","mdf","mszip","eoltxt",
                                "DICTTXT","BIGTEXT","NOWRT","tar","PNG8","PNG8G","IMPNG24","PNG24","IMPNG32","PNG32","Recursive","WIT","ISO9960","IS-CAB","SHRINK","REDUCE","IMPLODE"};

typedef enum {STREAM=0,HASINFO=1} Filetypes;

typedef enum {M_NO=-1,M_RECORD=0,M_IM8,M_IM24,M_SPARSE,M_JPEG,M_WAV,M_MATCH,M_MATCH1,M_DISTANCE,
              M_EXE,M_INDIRECT,M_DMC,M_NEST,M_NORMAL,M_IM1,M_XML,M_IM4,M_TEXT,
              M_WORD,M_DEC,M_LINEAR,M_SPARSEMATCH,M_SPARSE_Y,M_PPM,M_CHART,M_LSTM,M_MODEL_COUNT} ModelTypes;
static  std::string modelNames[M_MODEL_COUNT]={"RECORD","IM8","IM24","SPARSE","JPEG","WAV","MATCH","MATCH1","DISTANCE",
              "EXE","INDIRECT","DMC","NEST","NORMAL","IM1","XML","IM4","TEXT",
              "WORD","DEC","LINEAR","SPARSEMATCH","SPARSE_Y","M_PPM","M_CHART","M_LSTM"};
typedef enum {FCOMPRESS,FDECOMPRESS, FCOMPARE, FDISCARD,FEQUAL} FMode;

typedef enum {NONE=0,START,INFO,END,DATA,DISABLE,RESET,REQUEST,CUSTOM} DetectState; 
