/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "huffman.h"

static void table_insert(huffman_decoder_t *d, size_t sym, int len,
                         uint16_t codeword)
{
        int pad_len;
        uint16_t padding, index;

        assert(len <= HUFFMAN_LOOKUP_TABLE_BITS);

        codeword = reverse16(codeword, len); /* Make it LSB-first. */
        pad_len = HUFFMAN_LOOKUP_TABLE_BITS - len;

        /* Pad the pad_len upper bits with all bit combinations. */
        for (padding = 0; padding < (1U << pad_len); padding++) {
                index = (uint16_t)(codeword | (padding << len));
                d->table[index].sym = (uint16_t)sym;
                d->table[index].len = (uint16_t)len;

                assert(d->table[index].sym == sym && "Fits in bitfield.");
                assert(d->table[index].len == len && "Fits in bitfield.");
        }
}

bool huffman_decoder_init(huffman_decoder_t *d, const uint8_t *lengths,
                          size_t n)
{
        size_t i;
        uint16_t count[MAX_HUFFMAN_BITS + 1] = {0};
        uint16_t code[MAX_HUFFMAN_BITS + 1];
        uint32_t s;
        uint16_t sym_idx[MAX_HUFFMAN_BITS + 1];
        int l;

#ifndef NDEBUG
        assert(n <= MAX_HUFFMAN_SYMBOLS);
        d->num_syms = n;
#endif

        /* Zero-initialize the lookup table. */
        for (i = 0; i < sizeof(d->table) / sizeof(d->table[0]); i++) {
                d->table[i].len = 0;
        }

        /* Count the number of codewords of each length. */
        for (i = 0; i < n; i++) {
                assert(lengths[i] <= MAX_HUFFMAN_BITS);
                count[lengths[i]]++;
        }
        count[0] = 0;  /* Ignore zero-length codewords. */

        /* Compute sentinel_bits and offset_first_sym_idx for each length. */
        code[0] = 0;
        sym_idx[0] = 0;
        for (l = 1; l <= MAX_HUFFMAN_BITS; l++) {
                /* First canonical codeword of this length. */
                code[l] = (uint16_t)((code[l - 1] + count[l - 1]) << 1);

                if (count[l] != 0 && code[l] + count[l] - 1 > (1 << l) - 1) {
                        /* The last codeword is longer than l bits. */
                        return false;
                }

                s = (uint32_t)((code[l] + count[l]) << (MAX_HUFFMAN_BITS - l));
                d->sentinel_bits[l] = s;
                assert(d->sentinel_bits[l] >= code[l] && "No overflow!");

                sym_idx[l] = sym_idx[l - 1] + count[l - 1];
                d->offset_first_sym_idx[l] = sym_idx[l] - code[l];
        }

        /* Build mapping from index to symbol and populate the lookup table. */
        for (i = 0; i < n; i++) {
                l = lengths[i];
                if (l == 0) {
                        continue;
                }

                d->syms[sym_idx[l]] = (uint16_t)i;
                sym_idx[l]++;

                if (l <= HUFFMAN_LOOKUP_TABLE_BITS) {
                        table_insert(d, i, l, code[l]);
                        code[l]++;
                }
        }

        return true;
}

/* Swap the 32-bit values pointed to by a and b. */
static void swap32(uint32_t *a, uint32_t *b)
{
        uint32_t tmp;

        tmp = *a;
        *a = *b;
        *b = tmp;
}

/* Move element i in the n-element heap down to restore the minheap property. */
static void minheap_down(uint32_t *heap, size_t n, size_t i)
{
        size_t left, right, min;

        assert(i >= 1 && i <= n && "i must be inside the heap");

        /* While the i-th element has at least one child. */
        while (i * 2 <= n) {
                left = i * 2;
                right = i * 2 + 1;

                /* Find the child with lowest value. */
                min = left;
                if (right <= n && heap[right] < heap[left]) {
                        min = right;
                }

                /* Move i down if it is larger. */
                if (heap[min] < heap[i]) {
                        swap32(&heap[min], &heap[i]);
                        i = min;
                } else {
                        break;
                }
        }
}

/* Establish minheap property for heap[1..n]. */
static void minheap_heapify(uint32_t *heap, size_t n)
{
        size_t i;

        /* Floyd's algorithm. */
        for (i = n / 2; i >= 1; i--) {
                minheap_down(heap, n, i);
        }
}

/* Construct a Huffman code for n symbols with the frequencies in freq, and
 * codeword length limited to max_len. The sum of the frequencies must be <=
 * UINT16_MAX. max_len must be large enough that a code is always possible,
 * i.e. 2 ** max_len >= n. Symbols with zero frequency are not part of the code
 * and get length zero. Outputs the codeword lengths in lengths[0..n-1]. */
static void compute_huffman_lengths(const uint16_t *freqs, size_t n,
                                    uint8_t max_len, uint8_t *lengths)
{
        uint32_t nodes[MAX_HUFFMAN_SYMBOLS * 2 + 1], p, q;
        uint16_t freq;
        size_t i, h, l;
        uint16_t freq_shift = 0;

#ifndef NDEBUG
        uint32_t freq_sum = 0;
        for (i = 0; i < n; i++) {
                freq_sum += freqs[i];
        }
        assert(freq_sum <= UINT16_MAX && "Frequency sum too large!");
#endif

        assert(n <= MAX_HUFFMAN_SYMBOLS);
        assert((1U << max_len) >= n && "max_len must be large enough");

try_again:
        /* Initialize the heap. h is the heap size. */
        h = 0;
        for (i = 0; i < n; i++) {
                freq = freqs[i];

                if (freq == 0) {
                        continue; /* Ignore zero-frequency symbols. */
                }

                /* Shift frequencies down towards 1. */
                freq >>= freq_shift;
                if (freq == 0) {
                        freq = 1;
                }

                /* High 16 bits: Symbol frequency.
                   Low 16 bits:  Symbol link element index. */
                h++;
                nodes[h] = ((uint32_t)freq << 16) | (uint32_t)(n + h);
        }
        minheap_heapify(nodes, h);

        /* Special case for fewer than two non-zero symbols. */
        if (h < 2) {
                for (i = 0; i < n; i++) {
                        lengths[i] = (freqs[i] == 0) ? 0 : 1;
                }
                return;
        }

        /* Build the Huffman tree. */
        while (h > 1) {
                /* Remove the lowest frequency node p from the heap. */
                p = nodes[1];
                nodes[1] = nodes[h--];
                minheap_down(nodes, h, 1);

                /* Get q, the next lowest frequency node. */
                q = nodes[1];

                /* Replace q with a new symbol with the combined frequencies of
                   p and q, and with the no longer used h+1'th node as the
                   link element. */
                nodes[1] = ((p & 0xffff0000) + (q & 0xffff0000))
                           | (uint32_t)(h + 1);

                /* Set the links of p and q to point to the link element of
                   the new node. */
                nodes[p & 0xffff] = nodes[q & 0xffff] = (uint32_t)(h + 1);

                /* Move the new symbol down to restore heap property. */
                minheap_down(nodes, h, 1);
        }

        /* Compute the codeword length for each symbol. */
        h = 0;
        for (i = 0; i < n; i++) {
                if (freqs[i] == 0) {
                        lengths[i] = 0;
                        continue;
                }
                h++;

                /* Link element for the i-th symbol. */
                p = nodes[n + h];

                /* Follow the links until we hit the root (link index 2). */
                l = 1;
                while (p != 2) {
                        l++;
                        p = nodes[p];
                }

                if (l > max_len) {
                        /* Push freqs lower. */
                        assert(freq_shift != 15 && "freq_shift too high!");
                        freq_shift++;
                        goto try_again;
                }

                assert(l <= UINT8_MAX);
                lengths[i] = (uint8_t)l;
        }
}

static void compute_canonical_code(uint16_t *codewords, const uint8_t *lengths,
                                   size_t n)
{
        size_t i;
        uint16_t count[MAX_HUFFMAN_BITS + 1] = {0};
        uint16_t code[MAX_HUFFMAN_BITS + 1];
        int l;

        /* Count the number of codewords of each length. */
        for (i = 0; i < n; i++) {
                count[lengths[i]]++;
        }
        count[0] = 0; /* Ignore zero-length codes. */

        /* Compute the first codeword for each length. */
        code[0] = 0;
        for (l = 1; l <= MAX_HUFFMAN_BITS; l++) {
                code[l] = (uint16_t)((code[l - 1] + count[l - 1]) << 1);
        }

        /* Assign a codeword for each symbol. */
        for (i = 0; i < n; i++) {
                l = lengths[i];
                if (l == 0) {
                        continue;
                }

                codewords[i] = reverse16(code[l]++, l); /* Make it LSB-first. */
        }
}

void huffman_encoder_init(huffman_encoder_t *e, const uint16_t *freqs, size_t n,
                          uint8_t max_codeword_len)
{
        assert(n <= MAX_HUFFMAN_SYMBOLS);
        assert(max_codeword_len <= MAX_HUFFMAN_BITS);

        compute_huffman_lengths(freqs, n, max_codeword_len, e->lengths);
        compute_canonical_code(e->codewords, e->lengths, n);
}

void huffman_encoder_init2(huffman_encoder_t *e, const uint8_t *lengths,
                           size_t n)
{
        size_t i;

        for (i = 0; i < n; i++) {
                e->lengths[i] = lengths[i];
        }
        compute_canonical_code(e->codewords, e->lengths, n);
}
