#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <mem.h>
#include <algorithm>

typedef enum {T_ImageWidth=256,T_ImageLength,T_BitsPerSample,T_Compression,
              T_StripOffsets=273,
              T_SamplesPerPixel=277,
              T_StripByteCounts=279,
              T_SubIFDs=330,
              JPEGProc=513,
              JPEGInterchangeFormat=514,
              T_Exif_IFD=34665
} TIFFTags;
typedef struct TiffTag;
struct TiffTag {
	uint16_t  TagId;   // identifier
	uint16_t  Type;    // The scalar type of the data items
	uint32_t  Count;   // The number of items in the tag data
	uint32_t  Offset;  // The byte offset to the data items
	/*bool cmp(const TiffTag& a,const TiffTag& b)  {
        return a.Offset < a.Offset;
    }*/
};

struct TiffIFD {
	uint16_t    NumDirEntries;    // Number of Tags in IFD
	std::vector<TiffTag>  Tags;   // Tags array
	uint32_t   NextIFD;           // Offset to next IFD
	uint32_t   CurrentIFD;
};

// For detected images
struct detTIFF {
  uint32_t size;
  uint64_t offset;
  uint16_t compression;
  uint32_t width;
  uint32_t height;
  uint8_t bits;
  uint8_t bits1;
};

class TIFFParser: public Parser {
    std::vector<detTIFF> dtf;
    detTIFF idfImg;
    size_t count;
    uint64_t tiffi;
    bool tiffMM;
    uint64_t tiffImageStart;
    uint64_t tiffImageEnd;
    int tiffImages;
    uint64_t dirEntry;
    std::vector<TiffIFD> ifd;
    int tagsIn;
    int parseCount;
    uint8_t *tagT; // in buffer for reading tiff tags by one byte at the time
    int tagTx;
    std::vector<TiffTag> tagC;  // tag id contents to read
    uint64_t tagCx; // current read entry index
    uint64_t tagCs; // size
    uint64_t tagCc; // count
    uint64_t tagCi; // id
    uint8_t  *tagCd;
    uint32_t tagCdi;
    uint64_t relAdd=0;
    uint64_t info;
    uint32_t buf0, buf1, buf2, buf3;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    const std::string TIFFCompStr(int i);
    const std::string TIFFTypeStr(int i);
    const uint64_t NextTagContent(uint64_t x);
public:    
    TIFFParser();
    ~TIFFParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
