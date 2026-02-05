/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef TABLES_H
#define TABLES_H

#include <stdint.h>

/* Element x contains the value of x with the bits in reverse order. */
extern const uint8_t reverse8_tbl[UINT8_MAX + 1];

/* Code lengths for fixed Huffman coding of litlen and dist symbols. */
extern const uint8_t fixed_litlen_lengths[288];
extern const uint8_t fixed_dist_lengths[32];

/* Table of litlen symbol values minus 257 with corresponding base length
   and number of extra bits. */
struct litlen_tbl_t {
        uint16_t base_len : 9;
        uint16_t ebits : 7;
};
extern const struct litlen_tbl_t litlen_tbl[29];

/* Mapping from length (3--258) to litlen symbol (257--285). */
extern const uint16_t len2litlen[259];

/* Table of dist symbol values with corresponding base distance and number of
   extra bits. */
struct dist_tbl_t {
        uint16_t base_dist;
        uint16_t ebits;
};
extern const struct dist_tbl_t dist_tbl[30];

/* Mapping from distance to dist symbol, split in two tables. */
/* (distance - 1)      => dist, for distance 1--256. */
extern const uint8_t distance2dist_lo[256];
/* (distance - 1) >> 7 => dist, for distance 257--32768. */
extern const uint8_t distance2dist_hi[256];

/* Table for computing CRC-32. */
extern const uint32_t crc32_tbl[256];

#endif
