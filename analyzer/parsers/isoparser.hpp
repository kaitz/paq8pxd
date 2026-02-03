#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include <set>

// For IS9960 detection

// Expected sector size 2048
// File starts with 16 sectors (32 kb) of arbitrary data (boot, etc), most times it contains \0 (non-usb iso)
// This is followed by Volume Descriptors, each 1 sector.
//  First byte in this sector describes:
//  0     Boot Record
//  1     Primary Volume Descriptor
//  2     Supplementary Volume Descriptor
//  3     Volume Partition Descriptor
//  4-254 Reserved
//  255   Volume Descriptor Set Terminator
//
// We need to extract root directory location from Primary Volume Descriptor and
// ignore one in Supplementary Volume Descriptor if present.
// After that we can start extracting file names and their location inside the ISO image.
// We collect info about files and all sub-directorys. Parsed sub-directory is removed.
// If there are no more directories then we are done.
// Rock Ridge in directory entry is ignored (contains long file names)
// Extensions SUSP is ignored
//
// Overall we need file start and end location inside iso file.
// File extension is used to predefine next level parser.

typedef struct {uint8_t le[2];}        luint16;
typedef struct {uint8_t be[2];}        buint16;
typedef struct {uint8_t le[2], be[2];} duint16;
typedef struct {uint8_t le[4];}        luint32;
typedef struct {uint8_t be[4];}        buint32;
typedef struct {uint8_t le[4], be[4];} duint32;

// Directory entry
struct i9660_dir {
    uint8_t   length;
    uint8_t   xattr_length;
    duint32   sector;
    duint32   size;
    char      time[7];
    uint8_t   flags;
    uint8_t   unit_size;
    uint8_t   gap_size;
    duint16   vol_seq_number;
    uint8_t   name_len;
    char      name; // [name_len]
};
// Volume Descriptor
struct i9660_vd {
    uint8_t   type;
    char      magic[5];
    uint8_t   version;
    char      pad0[1];
    char      system_id[32];
    char      volume_id[32];
    char      pad1[8];
    duint32   volume_space_size;
    char      pad2[32];
    duint16   volume_set_size;
    duint16   volume_seq_number;
    duint16   logical_block_size;
    duint32   path_table_size;
    luint32   path_table_le;
    luint32   path_table_opt_le;
    buint32   path_table_be;
    buint32   path_table_opt_be;
    union {
     i9660_dir   root_dir;
     char        pad3[34];
    };
    char      volume_set_id[128];
    char      data_preparer_id[128];
    char      app_id[128];
    char      copyright_file[38];
    char      abstract_file[36];
    char      bibliography_file[37];
    char      volume_created[17];
    char      volume_modified[17];
    char      volume_expires[17];
    char      volume_effective[17];
    uint8_t   file_structure_version;
    char      pad4[1];
    char      app_reserved[512];
    char      reserved[653];
};

struct ISOfile {
    uint64_t start;
    uint64_t size;
    ParserType p;
};

class ISO9960Parser: public Parser { 
    uint64_t iso;
    i9660_vd isoVD;
    uint8_t sector[2048];
    uint32_t sectorpos;
    uint32_t sectcount;
    int rootdir,rootdirsup;
    std::set<uint32_t> sectorl;
    Array<ISOfile> isoF;
    uint64_t isoFiles;
    bool rec, volterm;
    uint64_t relAdd;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    ISO9960Parser();
    ~ISO9960Parser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
