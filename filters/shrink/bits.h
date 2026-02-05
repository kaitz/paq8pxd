/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef BITS_H
#define BITS_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "tables.h"

/* Reverse the n least significant bits of x.
   The (16 - n) most significant bits of the result will be zero. */
static inline uint16_t reverse16(uint16_t x, int n)
{
        uint16_t lo, hi;
        uint16_t reversed;

        assert(n > 0);
        assert(n <= 16);

        lo = x & 0xff;
        hi = x >> 8;

        reversed = (uint16_t)((reverse8_tbl[lo] << 8) | reverse8_tbl[hi]);

        return reversed >> (16 - n);
}

/* Read a 64-bit value from p in little-endian byte order. */
static inline uint64_t read64le(const uint8_t *p)
{
        /* The one true way, see
         * https://commandcenter.blogspot.com/2012/04/byte-order-fallacy.html */
        return ((uint64_t)p[0] << 0)  |
               ((uint64_t)p[1] << 8)  |
               ((uint64_t)p[2] << 16) |
               ((uint64_t)p[3] << 24) |
               ((uint64_t)p[4] << 32) |
               ((uint64_t)p[5] << 40) |
               ((uint64_t)p[6] << 48) |
               ((uint64_t)p[7] << 56);
}

static inline uint32_t read32le(const uint8_t *p)
{
        return ((uint32_t)p[0] << 0)  |
               ((uint32_t)p[1] << 8)  |
               ((uint32_t)p[2] << 16) |
               ((uint32_t)p[3] << 24);
}

static inline uint16_t read16le(const uint8_t *p)
{
        return (uint16_t)(
                ((uint16_t)p[0] << 0) |
                ((uint16_t)p[1] << 8));
}

/* Write a 64-bit value x to dst in little-endian byte order. */
static inline void write64le(uint8_t *dst, uint64_t x)
{
        dst[0] = (uint8_t)(x >> 0);
        dst[1] = (uint8_t)(x >> 8);
        dst[2] = (uint8_t)(x >> 16);
        dst[3] = (uint8_t)(x >> 24);
        dst[4] = (uint8_t)(x >> 32);
        dst[5] = (uint8_t)(x >> 40);
        dst[6] = (uint8_t)(x >> 48);
        dst[7] = (uint8_t)(x >> 56);
}

static inline void write32le(uint8_t *dst, uint32_t x)
{
        dst[0] = (uint8_t)(x >> 0);
        dst[1] = (uint8_t)(x >> 8);
        dst[2] = (uint8_t)(x >> 16);
        dst[3] = (uint8_t)(x >> 24);
}

static inline void write16le(uint8_t *dst, uint16_t x)
{
        dst[0] = (uint8_t)(x >> 0);
        dst[1] = (uint8_t)(x >> 8);
}

/* Get the n least significant bits of x. */
static inline uint64_t lsb(uint64_t x, size_t n)
{
        assert(n <= 63);
        return x & (((uint64_t)1 << n) - 1);
}

/* Round x up to the next multiple of m, which must be a power of 2. */
static inline size_t round_up(size_t x, size_t m)
{
        assert((m & (m - 1)) == 0 && "m must be a power of two");
        return (x + m - 1) & (size_t)(-m); /* Hacker's Delight (2nd), 3-1. */
}

#endif
