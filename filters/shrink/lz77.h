/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef LZ77_H
#define LZ77_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define LZ_WND_SIZE 32768

/* Perform LZ77 compression on the src_len bytes in src, with back references
   limited to a certain maximum distance and length, and with or without
   self-overlap. Returns false as soon as either of the callback functions
   returns false, otherwise returns true when all bytes have been processed. */
bool lz77_compress(const uint8_t *src, size_t src_len,
                   size_t max_dist, size_t max_len, bool allow_overlap,
                   bool (*lit_callback)(uint8_t lit, void *aux),
                   bool (*backref_callback)(size_t dist, size_t len, void *aux),
                   void *aux);

/* Output a literal byte at dst_pos in dst. */
static inline void lz77_output_lit(uint8_t *dst, size_t dst_pos, uint8_t lit)
{
        dst[dst_pos] = lit;
}

/* Output the (dist,len) back reference at dst_pos in dst. */
static inline void lz77_output_backref(uint8_t *dst, size_t dst_pos,
                                       size_t dist, size_t len)
{
        size_t i;

        assert(dist <= dst_pos && "cannot reference before beginning of dst");

        for (i = 0; i < len; i++) {
                dst[dst_pos] = dst[dst_pos - dist];
                dst_pos++;
        }
}

/* Output the (dist,len) backref at dst_pos in dst using 64-bit wide writes.
   There must be enough room for len bytes rounded to the next multiple of 8. */
static inline void lz77_output_backref64(uint8_t *dst, size_t dst_pos,
                                         size_t dist, size_t len)
{
        size_t i;
        uint64_t tmp;

        assert(len > 0);
        assert(dist <= dst_pos && "cannot reference before beginning of dst");

        if (len > dist) {
                /* Self-overlapping backref; fall back to byte-by-byte copy. */
                lz77_output_backref(dst, dst_pos, dist, len);
                return;
        }

        i = 0;
        do {
                memcpy(&tmp, &dst[dst_pos - dist + i], 8);
                memcpy(&dst[dst_pos + i], &tmp, 8);
                i += 8;
        } while (i < len);
}

#endif
