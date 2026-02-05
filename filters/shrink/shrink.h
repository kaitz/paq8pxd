/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef SHRINK_H
#define SHRINK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* PKZip Method 1: Shrink / Unshrink. */

/* Compress (shrink) the data in src into dst. The number of bytes output, at
   most dst_cap, is stored in *dst_used. Returns false if there is not enough
   room in dst. */
bool hwshrink(const uint8_t *src, size_t src_len,
              uint8_t *dst, size_t dst_cap, size_t *dst_used);


typedef enum {
        HWUNSHRINK_OK,   /* Unshrink was successful. */
        HWUNSHRINK_FULL, /* Not enough room in the output buffer. */
        HWUNSHRINK_ERR   /* Error in the input data. */
} unshrnk_stat_t;

/* Decompress (unshrink) the data in src. The number of input bytes used, at
   most src_len, is stored in *src_used on success. Output is written to dst.
   The number of bytes written, at most dst_cap, is stored in *dst_used on
   success. */
unshrnk_stat_t hwunshrink(const uint8_t *src, size_t src_len, size_t *src_used,
                          uint8_t *dst, size_t dst_cap, size_t *dst_used);

#endif
