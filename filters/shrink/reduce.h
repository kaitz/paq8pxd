/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef REDUCE_H
#define REDUCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* PKZip Methods 2--5: Reduce / Expand. */

/* Compress (reduce) the data in src into dst using the specified compression
   factor (1--4). The number of bytes output, at most dst_cap, is stored in
   *dst_used. Returns false if there is not enough room in dst. */
bool hwreduce(const uint8_t *src, size_t src_len, int comp_factor,
              uint8_t *dst, size_t dst_cap, size_t *dst_used);


typedef enum {
        HWEXPAND_OK,   /* Expand was successful. */
        HWEXPAND_ERR   /* Error in the input data. */
} expand_stat_t;

/* Decompress (expand) the data in src. The uncompressed data is uncomp_len
   bytes long and was compressed with comp_factor. The number of input bytes
   used, at most src_len, is written to *src_used on success. Output is written
   to dst. */
expand_stat_t hwexpand(const uint8_t *src, size_t src_len, size_t uncomp_len,
                       int comp_factor, size_t *src_used, uint8_t *dst);

#endif
