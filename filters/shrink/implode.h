/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef IMPLODE_H
#define IMPLODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* PKZip Method 6: Implode / Explode. */

/* Compress (implode) the data in src into dst, using a large window and Huffman
   coding of literals as specified by the flags. The number of bytes output, at
   most dst_cap, is stored in *dst_used. Returns false if there is not enough
   room in dst. */
bool hwimplode(const uint8_t *src, size_t src_len,
               bool large_wnd, bool lit_tree,
               uint8_t *dst, size_t dst_cap, size_t *dst_used);

typedef enum {
        HWEXPLODE_OK,   /* Explode was successful. */
        HWEXPLODE_ERR   /* Error in the input data. */
} explode_stat_t;

/* Decompress (explode) the data in src. The uncompressed data is uncomp_len
   bytes long. large_wnd is true if a large window was used for compression,
   lit_tree is true if literals were Huffman coded, and pk101_bug_compat is
   true if compatibility with PKZip 1.01/1.02 is desired. The number of input
   bytes used, at most src_len, is written to *src_used on success. Output is
   written to dst. */
explode_stat_t hwexplode(const uint8_t *src, size_t src_len, size_t uncomp_len,
                         bool large_wnd, bool lit_tree, bool pk101_bug_compat,
                         size_t *src_used, uint8_t *dst);

#endif
