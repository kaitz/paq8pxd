/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "bits.h"

/* Input bitstream. */
typedef struct istream_t istream_t;
struct istream_t {
        const uint8_t *src;  /* Source bytes. */
        const uint8_t *end;  /* Past-the-end byte of src. */
        size_t bitpos;       /* Position of the next bit to read. */
        size_t bitpos_end;   /* Position of past-the-end bit. */
};

/* Initialize an input stream to present the n bytes from src as an LSB-first
 * bitstream. */
static inline void istream_init(istream_t *is, const uint8_t *src, size_t n)
{
        is->src = src;
        is->end = src + n;
        is->bitpos = 0;
        is->bitpos_end = n * 8;
}

#define ISTREAM_MIN_BITS (64 - 7)

/* Get the next bits from the input stream. The number of bits returned is
 * between ISTREAM_MIN_BITS and 64, depending on the position in the stream, or
 * fewer if the end of stream is reached. The upper bits are zero-padded. */
static inline uint64_t istream_bits(const istream_t *is)
{
        const uint8_t *next;
        uint64_t bits;
        int i;

        next = &is->src[is->bitpos / 8];

        assert(next <= is->end && "Cannot read past end of stream.");

        if (is->end - next >= 8) {
                /* Common case: read 8 bytes in one go. */
                bits = read64le(next);
        } else {
                /* Read the available bytes and zero-pad. */
                bits = 0;
                for (i = 0; i < is->end - next; i++) {
                        bits |= (uint64_t)next[i] << (i * 8);
                }
        }

        return bits >> (is->bitpos % 8);
}

/* Advance n bits in the bitstream if possible. Returns false if that many bits
 * are not available in the stream. */
static inline bool istream_advance(istream_t *is, size_t n) {
        assert(is->bitpos <= is->bitpos_end);

        if (is->bitpos_end - is->bitpos < n) {
                return false;
        }

        is->bitpos += n;
        return true;
}

/* Align the input stream to the next 8-bit boundary and return a pointer to
 * that byte, which may be the past-the-end-of-stream byte. */
static inline const uint8_t *istream_byte_align(istream_t *is)
{
        const uint8_t *byte;

        assert(is->bitpos <= is->bitpos_end && "Not past end of stream.");

        is->bitpos = round_up(is->bitpos, 8);
        byte = &is->src[is->bitpos / 8];
        assert(byte <= is->end);

        return byte;
}

static inline size_t istream_bytes_read(istream_t *is)
{
        return round_up(is->bitpos, 8) / 8;
}


/* Output bitstream. */
typedef struct ostream_t ostream_t;
struct ostream_t {
        uint8_t *dst;
        uint8_t *end;
        size_t bitpos;
        size_t bitpos_end;
};

/* Initialize an output stream to write LSB-first bits into dst[0..n-1]. */
static inline void ostream_init(ostream_t *os, uint8_t *dst, size_t n)
{
        os->dst = dst;
        os->end = dst + n;
        os->bitpos = 0;
        os->bitpos_end = n * 8;
}

/* Get the current bit position in the stream. */
static inline size_t ostream_bit_pos(const ostream_t *os)
{
        return os->bitpos;
}

/* Return the number of bytes written to the output buffer. */
static inline size_t ostream_bytes_written(ostream_t *os)
{
        return round_up(os->bitpos, 8) / 8;
}

/* Write n bits to the output stream. Returns false if there is not enough room
 * at the destination. */
static inline bool ostream_write(ostream_t *os, uint64_t bits, size_t n)
{
        uint8_t *p;
        uint64_t x;
        size_t shift, i;

        assert(n <= 57);
        assert(bits <= ((uint64_t)1 << n) - 1 && "Must fit in n bits.");

        if (os->bitpos_end - os->bitpos < n) {
                /* Not enough room. */
                return false;
        }

        p = &os->dst[os->bitpos / 8];
        shift = os->bitpos % 8;

        if (os->end - p >= 8) {
                /* Common case: read and write 8 bytes in one go. */
                x = read64le(p);
                x = lsb(x, shift);
                x |= bits << shift;
                write64le(p, x);
        } else {
                /* Slow case: read/write as many bytes as are available. */
                x = 0;
                for (i = 0; i < (size_t)(os->end - p); i++) {
                        x |= (uint64_t)p[i] << (i * 8);
                }
                x = lsb(x, shift);
                x |= bits << shift;
                for (i = 0; i < (size_t)(os->end - p); i++) {
                        p[i] = (uint8_t)(x >> (i * 8));
                }
        }

        os->bitpos += n;

        return true;
}

/* Align the bitstream to the next byte boundary, then write the n bytes from
   src to it. Returns false if there is not enough room in the stream. */
static inline bool ostream_write_bytes_aligned(ostream_t *os,
                                               const uint8_t *src,
                                               size_t n)
{
        if (os->bitpos_end - round_up(os->bitpos, 8) < n * 8) {
                return false;
        }

        os->bitpos = round_up(os->bitpos, 8);
        memcpy(&os->dst[os->bitpos / 8], src, n);
        os->bitpos += n * 8;

        return true;
}

#endif
