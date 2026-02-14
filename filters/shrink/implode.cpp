/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "implode.h"
#include <string.h>
#include "bitstream.h"
#include "huffman.h"
#include "lz77.h"

#define BUFFER_CAP (32 * 1024)

typedef struct implode_state_t implode_state_t;
struct implode_state_t {
        bool large_wnd;
        bool lit_tree;

        struct {
                uint16_t dist;   /* Backref dist, or 0 for literals. */
                uint16_t litlen; /* Literal byte (dist=0) or backref length. */
        } buffer[BUFFER_CAP];
        size_t buffer_size;
        bool buffer_flushed;

        uint16_t lit_freqs[256];
        uint16_t dist_sym_freqs[64];
        uint16_t len_sym_freqs[64];

        ostream_t os;
        huffman_encoder_t lit_encoder;
        huffman_encoder_t len_encoder;
        huffman_encoder_t dist_encoder;
};

static size_t max_dist(bool large_wnd) {
        return large_wnd ? 8192 : 4096;
}

static size_t max_len(bool lit_tree) {
        return (lit_tree ? 3 : 2) + 63 + 255;
}

static int dist_sym(size_t dist, bool large_wnd)
{
        assert(dist >= 1);
        assert(dist <= max_dist(large_wnd));

        dist -= 1;

        return (int)(dist >> (large_wnd ? 7 : 6));
}

static int len_sym(size_t len, bool lit_tree)
{
        assert(len >= (lit_tree ? 3 : 2));
        assert(len <= max_len(lit_tree));

        len -= (lit_tree ? 3 : 2);

        if (len < 63) {
                return (int)len;
        }

        return 63; /* The remainder is in a separate byte. */
}

static bool write_lit(implode_state_t *s, uint8_t lit)
{
        /* Literal marker bit. */
        if (!ostream_write(&s->os, 0x1, 1)) {
                return false;
        }

        if (s->lit_tree) {
                /* Huffman coded literal. */
                return ostream_write(&s->os, s->lit_encoder.codewords[lit],
                                             s->lit_encoder.lengths[lit]);
        }

        /* Raw literal. */
        return ostream_write(&s->os, lit, 8);
}

static bool write_backref(implode_state_t *s, size_t dist, size_t len)
{
        int d, l;
        size_t num_dist_bits, extra_len;

        d = dist_sym(dist, s->large_wnd);
        l = len_sym(len, s->lit_tree);

        /* Backref marker bit. */
        if (!ostream_write(&s->os, 0x0, 1)) {
                return false;
        }

        /* Lower dist bits. */
        assert(dist >= 1);
        num_dist_bits = (s->large_wnd ? 7 : 6);
        if (!ostream_write(&s->os, lsb(dist - 1, num_dist_bits),
                                   num_dist_bits)) {
                return false;
        }

        /* Upper 6 dist bits, Huffman coded. */
        if (!ostream_write(&s->os, s->dist_encoder.codewords[d],
                                   s->dist_encoder.lengths[d])) {
                return false;
        }

        /* Huffman coded length. */
        if (!ostream_write(&s->os, s->len_encoder.codewords[l],
                                   s->len_encoder.lengths[l])) {
                return false;
        }

        if (l == 63) {
                /* Extra length byte. */
                extra_len = len - 63 - (s->lit_tree ? 3 : 2);
                assert(extra_len <= UINT8_MAX);
                if (!ostream_write(&s->os, extra_len, 8)) {
                        return false;
                }
        }

        return true;
}

static bool write_huffman_code(ostream_t *os, const uint8_t *codeword_lengths,
                               size_t num_syms)
{
        struct {
                uint8_t len;
                uint8_t num;
        } rle[256];
        size_t rle_size, i;

        assert(num_syms > 0);
        assert(num_syms <= sizeof(rle) / sizeof(rle[0]));

        /* Run-length encode the codeword lengths. */
        rle[0].len = codeword_lengths[0];
        rle[0].num = 1;
        rle_size = 1;

        for (i = 1; i < num_syms; i++) {
                if (rle[rle_size - 1].len == codeword_lengths[i] &&
                    rle[rle_size - 1].num < 16) {
                        rle[rle_size - 1].num++;
                        continue;
                }

                assert(rle_size < sizeof(rle) / sizeof(rle[0]));
                rle[rle_size  ].len = codeword_lengths[i];
                rle[rle_size++].num = 1;
        }

        /* Write the number of run-length encoded lengths. */
        assert(rle_size >= 1);
        if (!ostream_write(os, rle_size - 1, 8)) {
                return false;
        }

        /* Write the run-length encoded lengths. */
        for (i = 0; i < rle_size; i++) {
                assert(rle[i].num >= 1 && rle[i].num <= 16);
                assert(rle[i].len >= 1 && rle[i].len <= 16);
                if (!ostream_write(os, rle[i].len - 1, 4) ||
                    !ostream_write(os, rle[i].num - 1, 4)) {
                        return false;
                }
        }

        return true;
}

static void init_encoder(huffman_encoder_t *e, uint16_t *freqs, size_t n)
{
        size_t i, scale_factor;
        uint16_t freq_sum, zero_freqs;

        assert(BUFFER_CAP <= UINT16_MAX &&
               "Frequency sum must be guaranteed to fit in 16 bits.");

        freq_sum = 0;
        zero_freqs = 0;

        for (i = 0; i < n; i++) {
                freq_sum += freqs[i];
                zero_freqs += (freqs[i] == 0);
        }

        scale_factor = UINT16_MAX / (freq_sum + zero_freqs);
        assert(scale_factor >= 1);

        for (i = 0; i < n; i++) {
                if (freqs[i] == 0) {
                        /* The Huffman encoder was designed for Deflate, which
                         * excludes zero-frequency symbols from the code. That
                         * doesn't work with Implode, so enforce a minimum
                         * frequency of one. */
                        freqs[i] = 1;
                        continue;
                }

                /* Scale up to emphasise difference to the zero-freq symbols. */
                freqs[i] *= scale_factor;
                assert(freqs[i] >= 1);
        }

        huffman_encoder_init(e, freqs, n, /*max_codeword_len=*/ 16);

        /* Flip the bits to get the Implode-style canonical code. */
        for (i = 0; i < n; i++) {
                assert(e->lengths[i] >= 1);
                e->codewords[i] = (uint16_t)lsb(~e->codewords[i],
                                                e->lengths[i]);
        }
}

static bool flush_buffer(implode_state_t *s)
{
        size_t i;

        assert(!s->buffer_flushed);

        if (s->lit_tree) {
                init_encoder(&s->lit_encoder, s->lit_freqs, 256);
                if (!write_huffman_code(&s->os, s->lit_encoder.lengths, 256)) {
                        return false;
                }
        }

        init_encoder(&s->len_encoder, s->len_sym_freqs, 64);
        if (!write_huffman_code(&s->os, s->len_encoder.lengths, 64)) {
                return false;
        }

        init_encoder(&s->dist_encoder, s->dist_sym_freqs, 64);
        if (!write_huffman_code(&s->os, s->dist_encoder.lengths, 64)) {
                return false;
        }

        for (i = 0; i < s->buffer_size; i++) {
                if (s->buffer[i].dist == 0) {
                        if (!write_lit(s, (uint8_t)s->buffer[i].litlen)) {
                                return false;
                        }
                } else {
                        if (!write_backref(s, s->buffer[i].dist,
                                              s->buffer[i].litlen)) {
                                return false;
                        }
                }
        }

        s->buffer_flushed = true;

        return true;
}

static bool lit_callback(uint8_t lit, void *aux)
{
        implode_state_t *s = (implode_state_t *)aux;

        if (s->buffer_flushed) {
                return write_lit(s, lit);
        }

        assert(s->buffer_size < BUFFER_CAP);
        s->buffer[s->buffer_size  ].dist = 0;
        s->buffer[s->buffer_size++].litlen = lit;

        s->lit_freqs[lit]++;

        if (s->buffer_size == BUFFER_CAP) {
                return flush_buffer(s);
        }

        return true;
}

static bool backref_callback(size_t dist, size_t len, void *aux)
{
        implode_state_t *s = (implode_state_t *)aux;

        assert(dist >= 1);
        assert(dist <= max_dist(s->large_wnd));
        assert(len >= (s->lit_tree ? 3 : 2));
        assert(len <= max_len(s->lit_tree));

        if (s->buffer_flushed) {
                return write_backref(s, dist, len);
        }

        assert(s->buffer_size < BUFFER_CAP);
        s->buffer[s->buffer_size  ].dist = (uint16_t)dist;
        s->buffer[s->buffer_size++].litlen = (uint16_t)len;

        s->dist_sym_freqs[dist_sym(dist, s->large_wnd)]++;
        s->len_sym_freqs[len_sym(len, s->lit_tree)]++;

        if (s->buffer_size == BUFFER_CAP) {
                return flush_buffer(s);
        }

        return true;
}

bool hwimplode(const uint8_t *src, size_t src_len,
               bool large_wnd, bool lit_tree,
               uint8_t *dst, size_t dst_cap, size_t *dst_used)
{
        implode_state_t s;

        s.large_wnd = large_wnd;
        s.lit_tree = lit_tree;
        s.buffer_size = 0;
        s.buffer_flushed = false;
        memset(s.dist_sym_freqs, 0, sizeof(s.dist_sym_freqs));
        memset(s.len_sym_freqs, 0, sizeof(s.len_sym_freqs));
        memset(s.lit_freqs, 0, sizeof(s.lit_freqs));
        ostream_init(&s.os, dst, dst_cap);

        if (!lz77_compress(src, src_len, max_dist(large_wnd),
                           max_len(lit_tree), /*allow_overlap=*/true,
                           lit_callback, backref_callback, &s)) {
                return false;
        }

        if (!s.buffer_flushed && !flush_buffer(&s)) {
                return false;
        }

        *dst_used = ostream_bytes_written(&s.os);

        return true;
}

/* Initialize the Huffman decoder d with num_lens codeword lengths read from is.
   Returns false if the input is invalid. */
static bool read_huffman_code(istream_t *is, size_t num_lens,
                              huffman_decoder_t *d)
{
        uint8_t lens[256];
        uint8_t byte, codeword_len, run_length;
        size_t num_bytes, byte_idx, codeword_idx, i;
        uint16_t len_count[17] = {0};
        int32_t avail_codewords;
        bool ok;

        assert(num_lens <= sizeof(lens) / sizeof(lens[0]));

        /* Number of bytes representing the Huffman code. */
        byte = (uint8_t)lsb(istream_bits(is), 8);
        num_bytes = (size_t)byte + 1;
        if (!istream_advance(is, 8)) {
                return false;
        }

        codeword_idx = 0;
        for (byte_idx = 0; byte_idx < num_bytes; byte_idx++) {
                byte = (uint8_t)lsb(istream_bits(is), 8);
                if (!istream_advance(is, 8)) {
                        return false;
                }

                codeword_len = (byte & 0xf) + 1; /* Low four bits plus one. */
                run_length   = (byte >> 4)  + 1; /* High four bits plus one. */

                assert(codeword_len >= 1 && codeword_len <= 16);
                assert(codeword_len < sizeof(len_count) / sizeof(len_count[0]));
                len_count[codeword_len] += run_length;

                if (codeword_idx + run_length > num_lens) {
                        return false; /* Too many codeword lengths. */
                }
                for (i = 0; i < run_length; i++) {
                        assert(codeword_idx < num_lens);
                        lens[codeword_idx++] = codeword_len;
                }
        }

        assert(codeword_idx <= num_lens);
        if (codeword_idx < num_lens) {
                return false; /* Too few codeword lengths. */
        }

        /* Check that the Huffman tree is full. */
        avail_codewords = 1;
        for (i = 1; i <= 16; i++) {
                assert(avail_codewords >= 0);
                avail_codewords *= 2;
                avail_codewords -= len_count[i];
                if (avail_codewords < 0) {
                        /* Higher count than available codewords. */
                        return false;
                }
        }
        if (avail_codewords != 0) {
                /* Not all codewords were used. */
                return false;
        }

        ok = huffman_decoder_init(d, lens, num_lens);
        assert(ok && "The checks above mean the tree should be valid.");
        (void)ok;

        return true;
}

explode_stat_t hwexplode(const uint8_t *src, size_t src_len, size_t uncomp_len,
                         bool large_wnd, bool lit_tree, bool pk101_bug_compat,
                         size_t *src_used, uint8_t *dst)
{
        istream_t is;
        huffman_decoder_t lit_decoder, len_decoder, dist_decoder;
        size_t dst_pos, used, used_tot, dist, len, i;
        uint64_t bits;
        int sym, min_len;

        istream_init(&is, src, src_len);

        if (lit_tree) {
                if (!read_huffman_code(&is, 256, &lit_decoder)) {
                        return HWEXPLODE_ERR;
                }
        }
        if (!read_huffman_code(&is, 64, &len_decoder) ||
            !read_huffman_code(&is, 64, &dist_decoder)) {
                return HWEXPLODE_ERR;
        }

        if (pk101_bug_compat) {
                min_len = (large_wnd ? 3 : 2);
        } else {
                min_len = (lit_tree ? 3 : 2);
        }

        dst_pos = 0;
        while (dst_pos < uncomp_len) {
                bits = istream_bits(&is);

                if (lsb(bits, 1) == 0x1) {
                        /* Literal. */
                        bits >>= 1;
                        if (lit_tree) {
                                sym = huffman_decode(&lit_decoder,
                                                     (uint16_t)~bits, &used);
                                assert(sym >= 0 && "huffman decode successful");
                                if (!istream_advance(&is, 1 + used)) {
                                        return HWEXPLODE_ERR;
                                }
                        } else {
                                sym = (int)lsb(bits, 8);
                                if (!istream_advance(&is, 1 + 8)) {
                                        return HWEXPLODE_ERR;
                                }
                        }
                        assert(sym >= 0 && sym <= UINT8_MAX);
                        dst[dst_pos++] = (uint8_t)sym;
                        continue;
                }

                /* Backref. */
                assert(lsb(bits, 1) == 0x0);
                used_tot = 1;
                bits >>= 1;

                /* Read the low dist bits. */
                if (large_wnd) {
                        dist = (size_t)lsb(bits, 7);
                        bits >>= 7;
                        used_tot += 7;
                } else {
                        dist = (size_t)lsb(bits, 6);
                        bits >>= 6;
                        used_tot += 6;
                }

                /* Read the Huffman-encoded high dist bits. */
                sym = huffman_decode(&dist_decoder, (uint16_t)~bits, &used);
                assert(sym >= 0 && "huffman decode successful");
                used_tot += used;
                bits >>= used;
                dist |= (size_t)sym << (large_wnd ? 7 : 6);
                dist += 1;

                /* Read the Huffman-encoded len. */
                sym = huffman_decode(&len_decoder, (uint16_t)~bits, &used);
                assert(sym >= 0 && "huffman decode successful");
                used_tot += used;
                bits >>= used;
                len = (size_t)(sym + min_len);

                if (sym == 63) {
                        /* Read an extra len byte. */
                        len += (size_t)lsb(bits, 8);
                        used_tot += 8;
                        bits >>= 8;
                }
                assert(used_tot <= ISTREAM_MIN_BITS);
                if (!istream_advance(&is, used_tot)) {
                        return HWEXPLODE_ERR;
                }

                if (round_up(len, 8) <= uncomp_len - dst_pos &&
                    dist <= dst_pos) {
                        /* Enough room and no implicit zeros; chunked copy. */
                        lz77_output_backref64(dst, dst_pos, dist, len);
                        dst_pos += len;
                } else if (len > uncomp_len - dst_pos) {
                        /* Not enough room. */
                        return HWEXPLODE_ERR;
                } else {
                        /* Copy, handling overlap and implicit zeros. */
                        for (i = 0; i < len; i++) {
                                if (dist > dst_pos) {
                                        dst[dst_pos++] = 0;
                                        continue;
                                }
                                dst[dst_pos] = dst[dst_pos - dist];
                                dst_pos++;
                        }
                }
        }

        *src_used = istream_bytes_read(&is);
        return HWEXPLODE_OK;
}
