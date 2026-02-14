/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "bits.h"

#define MAX_HUFFMAN_SYMBOLS 288      /* Deflate uses max 288 symbols. */
#define MAX_HUFFMAN_BITS 16          /* Implode uses max 16-bit codewords. */
#define HUFFMAN_LOOKUP_TABLE_BITS 8  /* Seems a good trade-off. */

typedef struct huffman_decoder_t huffman_decoder_t;
struct huffman_decoder_t {
        /* Lookup table for fast decoding of short codewords. */
        struct {
                uint16_t sym : 9;  /* Wide enough to fit the max symbol nbr. */
                uint16_t len : 7;  /* 0 means no symbol. */
        } table[1U << HUFFMAN_LOOKUP_TABLE_BITS];

        /* "Sentinel bits" value for each codeword length. */
        uint32_t sentinel_bits[MAX_HUFFMAN_BITS + 1];

        /* First symbol index minus first codeword mod 2**16 for each length. */
        uint16_t offset_first_sym_idx[MAX_HUFFMAN_BITS + 1];

        /* Map from symbol index to symbol. */
        uint16_t syms[MAX_HUFFMAN_SYMBOLS];
#ifndef NDEBUG
        size_t num_syms;
#endif
};

/* Initialize huffman decoder d for a code defined by the n codeword lengths.
   Returns false if the codeword lengths do not correspond to a valid prefix
   code. */
bool huffman_decoder_init(huffman_decoder_t *d, const uint8_t *lengths,
                          size_t n);

/* Use the decoder d to decode a symbol from the LSB-first zero-padded bits.
 * Returns the decoded symbol number or -1 if no symbol could be decoded.
 * *num_used_bits will be set to the number of bits used to decode the symbol,
 * or zero if no symbol could be decoded. */
static inline int huffman_decode(const huffman_decoder_t *d, uint16_t bits,
                                 size_t *num_used_bits)
{
        uint64_t lookup_bits;
        size_t l;
        size_t sym_idx;

        /* First try the lookup table. */
        lookup_bits = lsb(bits, HUFFMAN_LOOKUP_TABLE_BITS);
        assert(lookup_bits < sizeof(d->table) / sizeof(d->table[0]));
        if (d->table[lookup_bits].len != 0) {
                assert(d->table[lookup_bits].len <= HUFFMAN_LOOKUP_TABLE_BITS);
                assert(d->table[lookup_bits].sym < d->num_syms);

                *num_used_bits = d->table[lookup_bits].len;
                return d->table[lookup_bits].sym;
        }

        /* Then do canonical decoding with the bits in MSB-first order. */
        bits = reverse16(bits, MAX_HUFFMAN_BITS);
        for (l = HUFFMAN_LOOKUP_TABLE_BITS + 1; l <= MAX_HUFFMAN_BITS; l++) {
                if (bits < d->sentinel_bits[l]) {
                        bits >>= MAX_HUFFMAN_BITS - l;

                        sym_idx = (uint16_t)(d->offset_first_sym_idx[l] + bits);
                        assert(sym_idx < d->num_syms);

                        *num_used_bits = l;
                        return d->syms[sym_idx];
                }
        }

        *num_used_bits = 0;
        return -1;
}


typedef struct huffman_encoder_t huffman_encoder_t;
struct huffman_encoder_t {
        uint16_t codewords[MAX_HUFFMAN_SYMBOLS]; /* LSB-first codewords. */
        uint8_t lengths[MAX_HUFFMAN_SYMBOLS];    /* Codeword lengths. */
};

/* Initialize a Huffman encoder based on the n symbol frequencies. */
void huffman_encoder_init(huffman_encoder_t *e, const uint16_t *freqs,
                          size_t n, uint8_t max_codeword_len);

/* Initialize a Huffman encoder based on the n codeword lengths. */
void huffman_encoder_init2(huffman_encoder_t *e, const uint8_t *lengths,
                           size_t n);

#endif
