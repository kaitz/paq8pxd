/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "shrink.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "bitstream.h"

#define MIN_CODE_SIZE   9
#define MAX_CODE_SIZE   13

#define MAX_CODE        ((1U << MAX_CODE_SIZE) - 1)
#define INVALID_CODE    UINT16_MAX
#define CONTROL_CODE    256
#define INC_CODE_SIZE   1
#define PARTIAL_CLEAR   2

#define HASH_BITS       (MAX_CODE_SIZE + 1) /* For a load factor of 0.5. */
#define HASHTAB_SIZE    (1U << HASH_BITS)
#define UNKNOWN_LEN     UINT16_MAX

/* Hash table where the keys are (prefix_code, ext_byte) pairs, and the values
 * are the corresponding code. If prefix_code is INVALID_CODE it means the hash
 * table slot is empty. */
typedef struct hashtab_t hashtab_t;
struct hashtab_t {
        uint16_t prefix_code;
        uint8_t  ext_byte;
        uint16_t code;
};

static void hashtab_init(hashtab_t *table)
{
        size_t i;

        for (i = 0; i < HASHTAB_SIZE; i++) {
                table[i].prefix_code = INVALID_CODE;
        }
}

static uint32_t hash(uint16_t code, uint8_t byte)
{
        static const uint32_t HASH_MUL = 2654435761U;

        /* Knuth's multiplicative hash. */
        return ((((uint32_t)byte << 16) | code) * HASH_MUL) >> (32 - HASH_BITS);
}

/* Return the code corresponding to a prefix code and extension byte if it
   exists in the table, or INVALID_CODE otherwise. */
static uint16_t hashtab_find(const hashtab_t *table, uint16_t prefix_code,
                             uint8_t ext_byte)
{
        size_t i = hash(prefix_code, ext_byte);

        assert(prefix_code != INVALID_CODE);

        while (true) {
                /* Scan until we find the key or an empty slot. */
                assert(i < HASHTAB_SIZE);
                if (table[i].prefix_code == prefix_code &&
                    table[i].ext_byte == ext_byte) {
                        return table[i].code;
                }
                if (table[i].prefix_code == INVALID_CODE) {
                        return INVALID_CODE;
                }
                i = (i + 1) % HASHTAB_SIZE;
                assert(i != hash(prefix_code, ext_byte));
        }
}

static void hashtab_insert(hashtab_t *table, uint16_t prefix_code,
                           uint8_t ext_byte, uint16_t code)
{
        size_t i = hash(prefix_code, ext_byte);

        assert(prefix_code != INVALID_CODE);
        assert(code != INVALID_CODE);
        assert(hashtab_find(table, prefix_code, ext_byte) == INVALID_CODE);

        while (true) {
                /* Scan until we find an empty slot. */
                assert(i < HASHTAB_SIZE);
                if (table[i].prefix_code == INVALID_CODE) {
                        break;
                }
                i = (i + 1) % HASHTAB_SIZE;
                assert(i != hash(prefix_code, ext_byte));
        }

        assert(i < HASHTAB_SIZE);
        table[i].code = code;
        table[i].prefix_code = prefix_code;
        table[i].ext_byte = ext_byte;

        assert(hashtab_find(table, prefix_code, ext_byte) == code);
}

typedef struct code_queue_t code_queue_t;
struct code_queue_t {
        uint16_t next_idx;
        uint16_t codes[MAX_CODE - CONTROL_CODE + 1];
};

static void code_queue_init(code_queue_t *q)
{
        size_t code_queue_size;
        uint16_t code;

        code_queue_size = 0;
        for (code = CONTROL_CODE + 1; code <= MAX_CODE; code++) {
                q->codes[code_queue_size++] = code;
        }
        assert(code_queue_size < sizeof(q->codes) / sizeof(q->codes[0]));
        q->codes[code_queue_size] = INVALID_CODE; /* End-of-queue marker. */
        q->next_idx = 0;
}

/* Return the next code in the queue, or INVALID_CODE if the queue is empty. */
static uint16_t code_queue_next(const code_queue_t *q)
{
        assert(q->next_idx < sizeof(q->codes) / sizeof(q->codes[0]));
        return q->codes[q->next_idx];
}

/* Return and remove the next code from the queue, or return INVALID_CODE if
   the queue is empty. */
static uint16_t code_queue_remove_next(code_queue_t *q)
{
        uint16_t code = code_queue_next(q);
        if (code != INVALID_CODE) {
                q->next_idx++;
        }
        return code;
}

/* Write a code to the output bitstream, increasing the code size if necessary.
   Returns true on success. */
static bool write_code(ostream_t *os, uint16_t code, size_t *code_size)
{
        assert(code <= MAX_CODE);

        while (code > (1U << *code_size) - 1) {
                /* Increase the code size. */
                assert(*code_size < MAX_CODE_SIZE);
                if (!ostream_write(os, CONTROL_CODE, *code_size) ||
                    !ostream_write(os, INC_CODE_SIZE, *code_size)) {
                        return false;
                }
                (*code_size)++;
        }

        return ostream_write(os, code, *code_size);
}

static void shrink_partial_clear(hashtab_t *hashtab, code_queue_t *queue)
{
        bool is_prefix[MAX_CODE + 1] = {0};
        hashtab_t new_hashtab[HASHTAB_SIZE];
        size_t i, code_queue_size;

        /* Scan for codes that have been used as a prefix. */
        for (i = 0; i < HASHTAB_SIZE; i++) {
                if (hashtab[i].prefix_code != INVALID_CODE) {
                        is_prefix[hashtab[i].prefix_code] = true;
                }
        }

        /* Build a new hash table with only the "prefix codes". */
        hashtab_init(new_hashtab);
        for (i = 0 ; i < HASHTAB_SIZE; i++) {
                if (hashtab[i].prefix_code == INVALID_CODE ||
                    !is_prefix[hashtab[i].code]) {
                        continue;
                }
                hashtab_insert(new_hashtab, hashtab[i].prefix_code,
                               hashtab[i].ext_byte, hashtab[i].code);
        }
        memcpy(hashtab, new_hashtab, sizeof(new_hashtab));

        /* Populate the queue with the "non-prefix" codes. */
        code_queue_size = 0;
        for (i = CONTROL_CODE + 1; i <= MAX_CODE; i++) {
                if (!is_prefix[i]) {
                        queue->codes[code_queue_size++] = (uint16_t)i;
                }
        }
        queue->codes[code_queue_size] = INVALID_CODE; /* End-of-queue marker. */
        queue->next_idx = 0;
}

bool hwshrink(const uint8_t *src, size_t src_len,
              uint8_t *dst, size_t dst_cap, size_t *dst_used)
{
        hashtab_t table[HASHTAB_SIZE];
        code_queue_t queue;
        ostream_t os;
        size_t code_size, i;
        uint8_t ext_byte;
        uint16_t curr_code, next_code, new_code;

        hashtab_init(table);
        code_queue_init(&queue);
        ostream_init(&os, dst, dst_cap);
        code_size = MIN_CODE_SIZE;

        if (src_len == 0) {
                *dst_used = 0;
                return true;
        }

        curr_code = src[0];

        for (i = 1; i < src_len; i++) {
                ext_byte = src[i];

                /* Search for a code with the current prefix + byte. */
                next_code = hashtab_find(table, curr_code, ext_byte);
                if (next_code != INVALID_CODE) {
                        curr_code = next_code;
                        continue;
                }

                /* Write out the current code. */
                if (!write_code(&os, curr_code, &code_size)) {
                        return false;
                }

                /* Assign a new code to the current prefix + byte. */
                new_code = code_queue_remove_next(&queue);
                if (new_code == INVALID_CODE) {
                        /* Try freeing up codes by partial clearing. */
                        shrink_partial_clear(table, &queue);
                        if (!ostream_write(&os, CONTROL_CODE, code_size) ||
                            !ostream_write(&os, PARTIAL_CLEAR, code_size)) {
                                return false;
                        }
                        new_code = code_queue_remove_next(&queue);
                }
                if (new_code != INVALID_CODE) {
                        hashtab_insert(table, curr_code, ext_byte, new_code);
                }

                /* Reset the parser starting at the byte. */
                curr_code = ext_byte;
        }

        /* Write out the last code. */
        if (!write_code(&os, curr_code, &code_size)) {
                return false;
        }

        *dst_used = ostream_bytes_written(&os);
        return true;
}


typedef struct codetab_t codetab_t;
struct codetab_t {
        uint16_t prefix_code;   /* INVALID_CODE means the entry is invalid. */
        uint8_t  ext_byte;
        uint16_t len;
        size_t   last_dst_pos;
};

static void codetab_init(codetab_t *codetab)
{
        size_t i;

        /* Codes for literal bytes. Set a phony prefix_code so they're valid. */
        for (i = 0; i <= UINT8_MAX; i++) {
                codetab[i].prefix_code = (uint16_t)i;
                codetab[i].ext_byte = (uint8_t)i;
                codetab[i].len = 1;
        }

        for (; i <= MAX_CODE; i++) {
                codetab[i].prefix_code = INVALID_CODE;
        }
}

static void unshrink_partial_clear(codetab_t *codetab, code_queue_t *queue)
{
        bool is_prefix[MAX_CODE + 1] = {0};
        size_t i, code_queue_size;

        /* Scan for codes that have been used as a prefix. */
        for (i = CONTROL_CODE + 1; i <= MAX_CODE; i++) {
                if (codetab[i].prefix_code != INVALID_CODE) {
                        is_prefix[codetab[i].prefix_code] = true;
                }
        }

        /* Clear "non-prefix" codes in the table; populate the code queue. */
        code_queue_size = 0;
        for (i = CONTROL_CODE + 1; i <= MAX_CODE; i++) {
                if (!is_prefix[i]) {
                        codetab[i].prefix_code = INVALID_CODE;
                        queue->codes[code_queue_size++] = (uint16_t)i;
                }
        }
        queue->codes[code_queue_size] = INVALID_CODE; /* End-of-queue marker. */
        queue->next_idx = 0;
}

/* Read the next code from the input stream and return it in next_code. Returns
   false if the end of the stream is reached. If the stream contains invalid
   data, next_code is set to INVALID_CODE but the return value is still true. */
static bool read_code(istream_t *is, size_t *code_size, codetab_t *codetab,
                      code_queue_t *queue, uint16_t *next_code)
{
        uint16_t code, control_code;

        assert(sizeof(code) * CHAR_BIT >= *code_size);

        code = (uint16_t)lsb(istream_bits(is), *code_size);
        if (!istream_advance(is, *code_size)) {
                return false;
        }

        /* Handle regular codes (the common case). */
        if (code != CONTROL_CODE) {
                *next_code = code;
                return true;
        }

        /* Handle control codes. */
        control_code = (uint16_t)lsb(istream_bits(is), *code_size);
        if (!istream_advance(is, *code_size)) {
                *next_code = INVALID_CODE;
                return true;
        }
        if (control_code == INC_CODE_SIZE && *code_size < MAX_CODE_SIZE) {
                (*code_size)++;
                return read_code(is, code_size, codetab, queue, next_code);
        }
        if (control_code == PARTIAL_CLEAR) {
                unshrink_partial_clear(codetab, queue);
                return read_code(is, code_size, codetab, queue, next_code);
        }
        *next_code = INVALID_CODE;
        return true;
}

/* Copy len bytes from dst[prev_pos] to dst[dst_pos]. */
static void copy_from_prev_pos(uint8_t *dst, size_t dst_cap,
                               size_t prev_pos, size_t dst_pos, size_t len)
{
        size_t i;
        uint64_t tmp;

        assert(dst_pos < dst_cap);
        assert(prev_pos < dst_pos);
        assert(len > 0);
        assert(len <= dst_cap - dst_pos);

        if (round_up(len, 8) > dst_cap - dst_pos) {
                /* Not enough room in dst for the sloppy copy below. */
                memmove(&dst[dst_pos], &dst[prev_pos], len);
                return;
        }

        if (prev_pos + len > dst_pos) {
                /* Benign one-byte overlap possible in the KwKwK case. */
                assert(prev_pos + len == dst_pos + 1);
                assert(dst[prev_pos] == dst[prev_pos + len - 1]);
        }

        i = 0;
        do {
                /* Sloppy copy: 64 bits at a time; a few extra don't matter. */
                memcpy(&tmp, &dst[prev_pos + i], 8);
                memcpy(&dst[dst_pos + i], &tmp, 8);
                i += 8;
        } while (i < len);
}

/* Output the string represented by a code into dst at dst_pos. Returns
 * HWUNSHRINK_OK on success, and also updates *first_byte and *len with the
 * first byte and length of the output string, respectively. */
static unshrnk_stat_t output_code(uint16_t code, uint8_t *dst, size_t dst_pos,
                                  size_t dst_cap, uint16_t prev_code,
                                  codetab_t *codetab, code_queue_t *queue,
                                  uint8_t *first_byte, size_t *len)
{
        uint16_t prefix_code;

        assert(code <= MAX_CODE && code != CONTROL_CODE);
        assert(dst_pos < dst_cap);

        if (code <= UINT8_MAX) {
                /* Output literal byte. */
                *first_byte = (uint8_t)code;
                *len = 1;
                dst[dst_pos] = (uint8_t)code;
                return HWUNSHRINK_OK;
        }

        if (codetab[code].prefix_code == INVALID_CODE ||
            codetab[code].prefix_code == code) {
                /* Reject invalid codes. Self-referential codes may exist in
                 * the table but cannot be used. */
                return HWUNSHRINK_ERR;
        }

        if (codetab[code].len != UNKNOWN_LEN) {
                /* Output string with known length (the common case). */
                if (dst_cap - dst_pos < codetab[code].len) {
                        return HWUNSHRINK_FULL;
                }
                copy_from_prev_pos(dst, dst_cap, codetab[code].last_dst_pos,
                                   dst_pos, codetab[code].len);
                *first_byte = dst[dst_pos];
                *len = codetab[code].len;
                return HWUNSHRINK_OK;
        }

        /* Output a string of unknown length. This happens when the prefix
           was invalid (due to partial clearing) when the code was inserted into
           the table. The prefix can then become valid when it's added to the
           table at a later point. */
        assert(codetab[code].len == UNKNOWN_LEN);
        prefix_code = codetab[code].prefix_code;
        assert(prefix_code > CONTROL_CODE);

        if (prefix_code == code_queue_next(queue)) {
                /* The prefix code hasn't been added yet, but we were just
                   about to: the KwKwK case. Add the previous string extended
                   with its first byte. */
                assert(codetab[prev_code].prefix_code != INVALID_CODE);
                codetab[prefix_code].prefix_code = prev_code;
                codetab[prefix_code].ext_byte = *first_byte;
                codetab[prefix_code].len = codetab[prev_code].len + 1;
                codetab[prefix_code].last_dst_pos =
                        codetab[prev_code].last_dst_pos;
                dst[dst_pos] = *first_byte;
        } else if (codetab[prefix_code].prefix_code == INVALID_CODE) {
                /* The prefix code is still invalid. */
                return HWUNSHRINK_ERR;
        }

        /* Output the prefix string, then the extension byte. */
        *len = codetab[prefix_code].len + 1;
        if (dst_cap - dst_pos < *len) {
                return HWUNSHRINK_FULL;
        }
        copy_from_prev_pos(dst, dst_cap, codetab[prefix_code].last_dst_pos,
                           dst_pos, codetab[prefix_code].len);
        dst[dst_pos + *len - 1] = codetab[code].ext_byte;
        *first_byte = dst[dst_pos];

        /* Update the code table now that the string has a length and pos. */
        assert(prev_code != code);
        codetab[code].len = (uint16_t)*len;
        codetab[code].last_dst_pos = dst_pos;

        return HWUNSHRINK_OK;
}

unshrnk_stat_t hwunshrink(const uint8_t *src, size_t src_len, size_t *src_used,
                          uint8_t *dst, size_t dst_cap, size_t *dst_used)
{
        codetab_t codetab[MAX_CODE + 1];
        code_queue_t queue;
        istream_t is;
        size_t code_size, dst_pos, i, len;
        uint16_t curr_code, prev_code, new_code, c;
        uint8_t first_byte;
        unshrnk_stat_t s;

        codetab_init(codetab);
        code_queue_init(&queue);
        istream_init(&is, src, src_len);
        code_size = MIN_CODE_SIZE;
        dst_pos = 0;

        /* Handle the first code separately since there is no previous code. */
        if (!read_code(&is, &code_size, codetab, &queue, &curr_code)) {
                *src_used = istream_bytes_read(&is);
                *dst_used = 0;
                return HWUNSHRINK_OK;
        }
        assert(curr_code != CONTROL_CODE);
        if (curr_code > UINT8_MAX) {
                return HWUNSHRINK_ERR; /* The first code must be a literal. */
        }
        if (dst_pos == dst_cap) {
                return HWUNSHRINK_FULL;
        }
        first_byte = (uint8_t)curr_code;
        dst[dst_pos] = (uint8_t)curr_code;
        codetab[curr_code].last_dst_pos = dst_pos;
        dst_pos++;

        prev_code = curr_code;
        while (read_code(&is, &code_size, codetab, &queue, &curr_code)) {
                if (curr_code == INVALID_CODE) {
                        return HWUNSHRINK_ERR;
                }
                if (dst_pos == dst_cap) {
                        return HWUNSHRINK_FULL;
                }

                /* Handle KwKwK: next code used before being added. */
                if (curr_code == code_queue_next(&queue)) {
                        if (codetab[prev_code].prefix_code == INVALID_CODE) {
                                /* The previous code is no longer valid. */
                                return HWUNSHRINK_ERR;
                        }
                        /* Extend the previous code with its first byte. */
                        assert(curr_code != prev_code);
                        codetab[curr_code].prefix_code = prev_code;
                        codetab[curr_code].ext_byte = first_byte;
                        codetab[curr_code].len = codetab[prev_code].len + 1;
                        codetab[curr_code].last_dst_pos =
                                codetab[prev_code].last_dst_pos;
                        assert(dst_pos < dst_cap);
                        dst[dst_pos] = first_byte;
                }

                /* Output the string represented by the current code. */
                s = output_code(curr_code, dst, dst_pos, dst_cap, prev_code,
                                codetab, &queue, &first_byte, &len);
                if (s != HWUNSHRINK_OK) {
                        return s;
                }

                /* Verify that the output matches walking the prefixes. */
                c = curr_code;
                for (i = 0; i < len; i++) {
                        assert(codetab[c].len == len - i);
                        assert(codetab[c].ext_byte ==
                               dst[dst_pos + len - i - 1]);
                        c = codetab[c].prefix_code;
                }

                /* Add a new code to the string table if there's room.
                   The string is the previous code's string extended with
                   the first byte of the current code's string. */
                new_code = code_queue_remove_next(&queue);
                if (new_code != INVALID_CODE) {
                        assert(codetab[prev_code].last_dst_pos < dst_pos);
                        codetab[new_code].prefix_code = prev_code;
                        codetab[new_code].ext_byte = first_byte;
                        codetab[new_code].len = codetab[prev_code].len + 1;
                        codetab[new_code].last_dst_pos =
                                codetab[prev_code].last_dst_pos;

                        if (codetab[prev_code].prefix_code == INVALID_CODE) {
                                /* prev_code was invalidated in a partial
                                 * clearing. Until that code is re-used, the
                                 * string represented by new_code is
                                 * indeterminate. */
                                codetab[new_code].len = UNKNOWN_LEN;
                        }
                        /* If prev_code was invalidated in a partial clearing,
                         * it's possible that new_code==prev_code, in which
                         * case it will never be used or cleared. */
                }

                codetab[curr_code].last_dst_pos = dst_pos;
                dst_pos += len;

                prev_code = curr_code;
        }

        *src_used = istream_bytes_read(&is);
        *dst_used = dst_pos;

        return HWUNSHRINK_OK;
}
