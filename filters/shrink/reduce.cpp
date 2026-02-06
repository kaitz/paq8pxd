/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "reduce.h"

#include <stdlib.h>
#include "bits.h"
#include "bitstream.h"
#include "lz77.h"

/* Number of bits used to represent indices in a follower set of size n. */
static uint8_t follower_idx_bw(size_t n)
{
        assert(n <= 32);

        if (n > 16) { return 5; }
        if (n > 8)  { return 4; }
        if (n > 4)  { return 3; }
        if (n > 2)  { return 2; }
        if (n > 0)  { return 1; }
        return 0;
}

typedef struct follower_set_t follower_set_t;
struct follower_set_t {
        uint8_t size;
        uint8_t idx_bw;
        uint8_t followers[32];
};

/* Read the follower sets from is into fsets. Returns true on success. */
static bool read_follower_sets(istream_t *is, follower_set_t *fsets)
{
        int i, j;
        uint8_t n;

        for (i = 255; i >= 0; i--) {
                n = (uint8_t)lsb(istream_bits(is), 6);
                if (n > 32) {
                        return false;
                }
                if (!istream_advance(is, 6)) {
                        return false;
                }
                fsets[i].size = n;
                fsets[i].idx_bw = follower_idx_bw(n);

                for (j = 0; j < fsets[i].size; j++) {
                        fsets[i].followers[j] = (uint8_t)istream_bits(is);
                        if (!istream_advance(is, 8)) {
                                return false;
                        }
                }
        }

        return true;
}

/* Read the next byte from is, decoded based on prev_byte and the follower sets.
   The byte is returned in *out_byte. The function returns true on success,
   and false on bad data or end of input. */
static bool read_next_byte(istream_t *is, uint8_t prev_byte,
                           const follower_set_t *fsets, uint8_t *out_byte)
{
        uint64_t bits;
        uint8_t idx_bw, follower_idx;

        bits = istream_bits(is);

        if (fsets[prev_byte].size == 0) {
                /* No followers; read a literal byte. */
                *out_byte = (uint8_t)bits;
                return istream_advance(is, 8);
        }

        if (lsb(bits, 1) == 1) {
                /* Don't use the follower set; read a literal byte. */
                *out_byte = (uint8_t)(bits >> 1);
                return istream_advance(is, 1 + 8);
        }

        /* The bits represent the index of a follower byte. */
        idx_bw = fsets[prev_byte].idx_bw;
        follower_idx = (uint8_t)lsb(bits >> 1, idx_bw);
        if (follower_idx >= fsets[prev_byte].size) {
                return false;
        }
        *out_byte = fsets[prev_byte].followers[follower_idx];
        return istream_advance(is, 1 + idx_bw);
}

static size_t max_len(int comp_factor)
{
        size_t v_len_bits = (size_t)(8 - comp_factor);

        assert(comp_factor >= 1 && comp_factor <= 4);

        /* Bits in V + extra len byte + implicit 3. */
        return ((1U << v_len_bits) - 1) + 255 + 3;
}

static size_t max_dist(int comp_factor)
{
        size_t v_dist_bits = (size_t)(comp_factor);

        assert(comp_factor >= 1 && comp_factor <= 4);

        /* Bits in V * 256 + W byte + implicit 1. */
        return ((1U << v_dist_bits) - 1) * 256 + 255 + 1;
}

#define DLE_BYTE 144

expand_stat_t hwexpand(const uint8_t *src, size_t src_len, size_t uncomp_len,
                       int comp_factor, size_t *src_used, uint8_t *dst)
{
        istream_t is;
        follower_set_t fsets[256];
        size_t v_len_bits, dst_pos, len, dist, i;
        uint8_t curr_byte, v;

        assert(comp_factor >= 1 && comp_factor <= 4);

        istream_init(&is, src, src_len);
        if (!read_follower_sets(&is, fsets)) {
                return HWEXPAND_ERR;
        }

        /* Number of bits in V used for backref length. */
        v_len_bits = (size_t)(8 - comp_factor);

        dst_pos = 0;
        curr_byte = 0; /* The first "previous byte" is implicitly zero. */

        while (dst_pos < uncomp_len) {
                /* Read a literal byte or DLE marker. */
                if (!read_next_byte(&is, curr_byte, fsets, &curr_byte)) {
                        return HWEXPAND_ERR;
                }
                if (curr_byte != DLE_BYTE) {
                        /* Output a literal byte. */
                        dst[dst_pos++] = curr_byte;
                        continue;
                }

                /* Read the V byte which determines the length. */
                if (!read_next_byte(&is, curr_byte, fsets, &curr_byte)) {
                        return HWEXPAND_ERR;
                }
                if (curr_byte == 0) {
                        /* Output a literal DLE byte. */
                        dst[dst_pos++] = DLE_BYTE;
                        continue;
                }
                v = curr_byte;
                len = (size_t)lsb(v, v_len_bits);
                if (len == (1U << v_len_bits) - 1) {
                        /* Read an extra length byte. */
                        if (!read_next_byte(&is, curr_byte, fsets,
                                            &curr_byte)) {
                                return HWEXPAND_ERR;
                        }
                        len += curr_byte;
                }
                len += 3;

                /* Read the W byte, which together with V gives the distance. */
                if (!read_next_byte(&is, curr_byte, fsets, &curr_byte)) {
                        return HWEXPAND_ERR;
                }
                dist = (v >> v_len_bits) * 256 + curr_byte + 1;

                assert(len <= max_len(comp_factor));
                assert(dist <= max_dist(comp_factor));

                /* Output the back reference. */
                if (round_up(len, 8) <= uncomp_len - dst_pos &&
                    dist <= dst_pos) {
                        /* Enough room and no implicit zeros; chunked copy. */
                        lz77_output_backref64(dst, dst_pos, dist, len);
                        dst_pos += len;
                } else if (len > uncomp_len - dst_pos) {
                        /* Not enough room. */
                        return HWEXPAND_ERR;
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

        return HWEXPAND_OK;
}

#define RAW_BYTES_SZ    (64 * 1024)
#define NO_FOLLOWER_IDX UINT8_MAX

typedef struct reduce_state_t reduce_state_t;
struct reduce_state_t {
        ostream_t os;
        int comp_factor;
        uint8_t prev_byte;
        bool raw_bytes_flushed;

        /* Raw bytes buffer. */
        uint8_t raw_bytes[RAW_BYTES_SZ];
        size_t num_raw_bytes;

        /* Map from (prev_byte,curr_byte) to follower_idx or NO_FOLLOWER_IDX. */
        uint8_t follower_idx[256][256];
        uint8_t follower_idx_bw[256];
};

struct follower {
        uint8_t byte;
        size_t count;
};

static int follower_cmp(const void *a, const void *b)
{
        const struct follower *l = (follower *)a;
        const struct follower *r = (follower *)b;

        /* Sort descending by count. */
        if (l->count < r->count) { return  1; }
        if (l->count > r->count) { return -1; }

        /* Break ties by sorting ascending by byte. */
        if (l->byte > r->byte) { return  1; }
        if (l->byte < r->byte) { return -1; }

        assert(l->count == r->count && l->byte == r->byte);
        return 0;
}

/* The cost in bits for writing the follower bytes using follower set size n. */
static size_t followers_cost(const struct follower *followers, size_t n)
{
        size_t cost, i;

        /* Cost for storing the follower set. */
        cost = n * 8;

        /* Cost for follower bytes in the set. */
        for (i = 0; i < n; i++) {
                cost += followers[i].count * (1 + follower_idx_bw(n));
        }
        /* Cost for follower bytes not in the set. */
        for (; i < 256; i++) {
                if (n == 0) {
                        cost += followers[i].count * 8;
                } else {
                        cost += followers[i].count * (1 + 8);
                }
        }

        return cost;
}

/* Compute and write the follower sets based on the raw bytes buffer. */
static bool write_follower_sets(reduce_state_t *s)
{
        size_t follower_count[256][256] = {{0}};
        struct follower followers[256];
        int prev_byte, curr_byte;
        size_t i, cost, min_cost, min_cost_size;

        /* Count followers. */
        prev_byte = 0;
        for (i = 0; i < s->num_raw_bytes; i++) {
                curr_byte = s->raw_bytes[i];
                follower_count[prev_byte][curr_byte]++;
                prev_byte = curr_byte;
        }

        for (curr_byte = UINT8_MAX; curr_byte >= 0; curr_byte--) {
                /* Initialize follower indices to invalid. */
                for (i = 0; i <= UINT8_MAX; i++) {
                        s->follower_idx[curr_byte][i] = NO_FOLLOWER_IDX;
                }

                /* Sort the followers for curr_byte. */
                for (i = 0; i <= UINT8_MAX; i++) {
                        followers[i].byte = (uint8_t)i;
                        followers[i].count = follower_count[curr_byte][i];
                }
                qsort(followers, 256, sizeof(followers[0]), follower_cmp);

                /* Find the follower set size with the lowest cost. */
                min_cost_size = 0;
                min_cost = followers_cost(followers, 0);
                for (i = 1; i <= 32; i++) {
                        cost = followers_cost(followers, i);
                        if (cost < min_cost) {
                                min_cost_size = i;
                                min_cost = cost;
                        }
                }

                /* Save the follower indices. */
                for (i = 0; i < min_cost_size; i++) {
                        s->follower_idx[curr_byte][followers[i].byte] =
                                (uint8_t)i;
                }
                s->follower_idx_bw[curr_byte] = follower_idx_bw(min_cost_size);

                /* Write the followers. */
                if (!ostream_write(&s->os, min_cost_size, 6)) {
                        return false;
                }
                for (i = 0; i < min_cost_size; i++) {
                        if (!ostream_write(&s->os, followers[i].byte, 8)) {
                                return false;
                        }
                }
        }

        return true;
}

static bool write_byte(reduce_state_t *s, uint8_t byte);

static bool flush_raw_bytes(reduce_state_t *s)
{
        size_t i;

        s->raw_bytes_flushed = true;

        if (!write_follower_sets(s)) {
                return false;
        }

        for (i = 0; i < s->num_raw_bytes; i++) {
                if (!write_byte(s, s->raw_bytes[i])) {
                        return false;
                }
        }

        return true;
}

static bool write_byte(reduce_state_t *s, uint8_t byte)
{
        uint8_t follower_idx, follower_idx_bw;

        if (!s->raw_bytes_flushed) {
                /* Accumulate bytes which will be used for computing the
                   follower sets. */
                assert(s->num_raw_bytes < RAW_BYTES_SZ);
                s->raw_bytes[s->num_raw_bytes++] = byte;

                if (s->num_raw_bytes == RAW_BYTES_SZ) {
                        /* Write follower sets and flush the bytes. */
                        return flush_raw_bytes(s);
                }

                return true;
        }

        follower_idx = s->follower_idx[s->prev_byte][byte];
        follower_idx_bw = s->follower_idx_bw[s->prev_byte];
        s->prev_byte = byte;

        if (follower_idx != NO_FOLLOWER_IDX) {
                /* Write (LSB-first) a 0 bit followed by the follower index. */
                return ostream_write(&s->os,
                                     (uint64_t)follower_idx << 1,
                                     follower_idx_bw + 1);
        }

        if (follower_idx_bw != 0) {
                /* Not using the follower set.
                   Write (LSB-first) a 1 bit followed by the literal byte. */
                return ostream_write(&s->os, (uint64_t)(byte << 1) | 0x1, 9);
        }

        /* No follower set; write the literal byte. */
        return ostream_write(&s->os, byte, 8);
}

static bool lit_callback(uint8_t lit, void *aux)
{
        reduce_state_t *s = (reduce_state_t *)aux;

        if (!write_byte(s, lit)) {
                return false;
        }

        if (lit == DLE_BYTE) {
                return write_byte(s, 0);
        }

        return true;
}

static inline size_t min(size_t a, size_t b)
{
        return a < b ? a : b;
}

static bool backref_callback(size_t dist, size_t len, void *aux)
{
        reduce_state_t *s = (reduce_state_t *)aux;
        size_t v_len_bits = (size_t)(8 - s->comp_factor);
        uint8_t v, elb, w;

        assert(len  >= 3 && len  <= max_len(s->comp_factor));
        assert(dist >= 1 && dist <= max_dist(s->comp_factor));

        assert(len <= dist && "Backref shouldn't self-overlap.");

        /* The implicit part of len and dist are not encoded. */
        len -= 3;
        dist -= 1;

        /* Write the DLE marker. */
        if (!write_byte(s, DLE_BYTE)) {
                return false;
        }

        /* Write V. */
        v = (uint8_t)min(len, (1U << v_len_bits) - 1);
        assert(dist / 256 <= (1U << s->comp_factor) - 1);
        v |= (dist / 256) << v_len_bits;
        assert(v != 0 && "The byte following DLE must be non-zero.");
        if (!write_byte(s, v)) {
                return false;
        }

        if (len >= (1U << v_len_bits) - 1) {
                /* Write extra length byte. */
                assert(len - ((1U << v_len_bits) - 1) <= UINT8_MAX);
                elb = (uint8_t)(len - ((1U << v_len_bits) - 1));
                if (!write_byte(s, elb)) {
                        return false;
                }
        }

        /* Write W. */
        w = (uint8_t)(dist % 256);
        if (!write_byte(s, w)) {
                return false;
        }

        return true;
}

bool hwreduce(const uint8_t *src, size_t src_len, int comp_factor,
              uint8_t *dst, size_t dst_cap, size_t *dst_used)
{
        reduce_state_t s;

        ostream_init(&s.os, dst, dst_cap);
        s.comp_factor = comp_factor;
        s.prev_byte = 0;
        s.raw_bytes_flushed = false;
        s.num_raw_bytes = 0;

        if (!lz77_compress(src, src_len,
                           max_dist(comp_factor), max_len(comp_factor),
                           /*allow_overlap=*/ false,
                           lit_callback, backref_callback, &s)) {
                return false;
        }

        if (!s.raw_bytes_flushed && !flush_raw_bytes(&s)) {
                return false;
        }

        *dst_used = ostream_bytes_written(&s.os);

        return true;
}
