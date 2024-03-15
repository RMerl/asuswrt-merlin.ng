/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

/* Original code from Argon2 reference source code package used under CC0
 * https://github.com/P-H-C/phc-winner-argon2
 * Copyright 2015
 * Daniel Dinu, Dmitry Khovratovich, Jean-Philippe Aumasson, and Samuel Neves
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "blake2.h"
#include "hashx_endian.h"

static const uint64_t blake2b_IV[8] = {
	UINT64_C(0x6a09e667f3bcc908), UINT64_C(0xbb67ae8584caa73b),
	UINT64_C(0x3c6ef372fe94f82b), UINT64_C(0xa54ff53a5f1d36f1),
	UINT64_C(0x510e527fade682d1), UINT64_C(0x9b05688c2b3e6c1f),
	UINT64_C(0x1f83d9abfb41bd6b), UINT64_C(0x5be0cd19137e2179) };

#define BLAKE2_SIGMA_0_0 0
#define BLAKE2_SIGMA_0_1 1
#define BLAKE2_SIGMA_0_2 2
#define BLAKE2_SIGMA_0_3 3
#define BLAKE2_SIGMA_0_4 4
#define BLAKE2_SIGMA_0_5 5
#define BLAKE2_SIGMA_0_6 6
#define BLAKE2_SIGMA_0_7 7
#define BLAKE2_SIGMA_0_8 8
#define BLAKE2_SIGMA_0_9 9
#define BLAKE2_SIGMA_0_10 10
#define BLAKE2_SIGMA_0_11 11
#define BLAKE2_SIGMA_0_12 12
#define BLAKE2_SIGMA_0_13 13
#define BLAKE2_SIGMA_0_14 14
#define BLAKE2_SIGMA_0_15 15

#define BLAKE2_SIGMA_1_0 14
#define BLAKE2_SIGMA_1_1 10
#define BLAKE2_SIGMA_1_2 4
#define BLAKE2_SIGMA_1_3 8
#define BLAKE2_SIGMA_1_4 9
#define BLAKE2_SIGMA_1_5 15
#define BLAKE2_SIGMA_1_6 13
#define BLAKE2_SIGMA_1_7 6
#define BLAKE2_SIGMA_1_8 1
#define BLAKE2_SIGMA_1_9 12
#define BLAKE2_SIGMA_1_10 0
#define BLAKE2_SIGMA_1_11 2
#define BLAKE2_SIGMA_1_12 11
#define BLAKE2_SIGMA_1_13 7
#define BLAKE2_SIGMA_1_14 5
#define BLAKE2_SIGMA_1_15 3

#define BLAKE2_SIGMA_2_0 11
#define BLAKE2_SIGMA_2_1 8
#define BLAKE2_SIGMA_2_2 12
#define BLAKE2_SIGMA_2_3 0
#define BLAKE2_SIGMA_2_4 5
#define BLAKE2_SIGMA_2_5 2
#define BLAKE2_SIGMA_2_6 15
#define BLAKE2_SIGMA_2_7 13
#define BLAKE2_SIGMA_2_8 10
#define BLAKE2_SIGMA_2_9 14
#define BLAKE2_SIGMA_2_10 3
#define BLAKE2_SIGMA_2_11 6
#define BLAKE2_SIGMA_2_12 7
#define BLAKE2_SIGMA_2_13 1
#define BLAKE2_SIGMA_2_14 9
#define BLAKE2_SIGMA_2_15 4

#define BLAKE2_SIGMA_3_0 7
#define BLAKE2_SIGMA_3_1 9
#define BLAKE2_SIGMA_3_2 3
#define BLAKE2_SIGMA_3_3 1
#define BLAKE2_SIGMA_3_4 13
#define BLAKE2_SIGMA_3_5 12
#define BLAKE2_SIGMA_3_6 11
#define BLAKE2_SIGMA_3_7 14
#define BLAKE2_SIGMA_3_8 2
#define BLAKE2_SIGMA_3_9 6
#define BLAKE2_SIGMA_3_10 5
#define BLAKE2_SIGMA_3_11 10
#define BLAKE2_SIGMA_3_12 4
#define BLAKE2_SIGMA_3_13 0
#define BLAKE2_SIGMA_3_14 15
#define BLAKE2_SIGMA_3_15 8

#define BLAKE2_SIGMA_4_0 9
#define BLAKE2_SIGMA_4_1 0
#define BLAKE2_SIGMA_4_2 5
#define BLAKE2_SIGMA_4_3 7
#define BLAKE2_SIGMA_4_4 2
#define BLAKE2_SIGMA_4_5 4
#define BLAKE2_SIGMA_4_6 10
#define BLAKE2_SIGMA_4_7 15
#define BLAKE2_SIGMA_4_8 14
#define BLAKE2_SIGMA_4_9 1
#define BLAKE2_SIGMA_4_10 11
#define BLAKE2_SIGMA_4_11 12
#define BLAKE2_SIGMA_4_12 6
#define BLAKE2_SIGMA_4_13 8
#define BLAKE2_SIGMA_4_14 3
#define BLAKE2_SIGMA_4_15 13

#define BLAKE2_SIGMA_5_0 2
#define BLAKE2_SIGMA_5_1 12
#define BLAKE2_SIGMA_5_2 6
#define BLAKE2_SIGMA_5_3 10
#define BLAKE2_SIGMA_5_4 0
#define BLAKE2_SIGMA_5_5 11
#define BLAKE2_SIGMA_5_6 8
#define BLAKE2_SIGMA_5_7 3
#define BLAKE2_SIGMA_5_8 4
#define BLAKE2_SIGMA_5_9 13
#define BLAKE2_SIGMA_5_10 7
#define BLAKE2_SIGMA_5_11 5
#define BLAKE2_SIGMA_5_12 15
#define BLAKE2_SIGMA_5_13 14
#define BLAKE2_SIGMA_5_14 1
#define BLAKE2_SIGMA_5_15 9

#define BLAKE2_SIGMA_6_0 12
#define BLAKE2_SIGMA_6_1 5
#define BLAKE2_SIGMA_6_2 1
#define BLAKE2_SIGMA_6_3 15
#define BLAKE2_SIGMA_6_4 14
#define BLAKE2_SIGMA_6_5 13
#define BLAKE2_SIGMA_6_6 4
#define BLAKE2_SIGMA_6_7 10
#define BLAKE2_SIGMA_6_8 0
#define BLAKE2_SIGMA_6_9 7
#define BLAKE2_SIGMA_6_10 6
#define BLAKE2_SIGMA_6_11 3
#define BLAKE2_SIGMA_6_12 9
#define BLAKE2_SIGMA_6_13 2
#define BLAKE2_SIGMA_6_14 8
#define BLAKE2_SIGMA_6_15 11

#define BLAKE2_SIGMA_7_0 13
#define BLAKE2_SIGMA_7_1 11
#define BLAKE2_SIGMA_7_2 7
#define BLAKE2_SIGMA_7_3 14
#define BLAKE2_SIGMA_7_4 12
#define BLAKE2_SIGMA_7_5 1
#define BLAKE2_SIGMA_7_6 3
#define BLAKE2_SIGMA_7_7 9
#define BLAKE2_SIGMA_7_8 5
#define BLAKE2_SIGMA_7_9 0
#define BLAKE2_SIGMA_7_10 15
#define BLAKE2_SIGMA_7_11 4
#define BLAKE2_SIGMA_7_12 8
#define BLAKE2_SIGMA_7_13 6
#define BLAKE2_SIGMA_7_14 2
#define BLAKE2_SIGMA_7_15 10

#define BLAKE2_SIGMA_8_0 6
#define BLAKE2_SIGMA_8_1 15
#define BLAKE2_SIGMA_8_2 14
#define BLAKE2_SIGMA_8_3 9
#define BLAKE2_SIGMA_8_4 11
#define BLAKE2_SIGMA_8_5 3
#define BLAKE2_SIGMA_8_6 0
#define BLAKE2_SIGMA_8_7 8
#define BLAKE2_SIGMA_8_8 12
#define BLAKE2_SIGMA_8_9 2
#define BLAKE2_SIGMA_8_10 13
#define BLAKE2_SIGMA_8_11 7
#define BLAKE2_SIGMA_8_12 1
#define BLAKE2_SIGMA_8_13 4
#define BLAKE2_SIGMA_8_14 10
#define BLAKE2_SIGMA_8_15 5

#define BLAKE2_SIGMA_9_0 10
#define BLAKE2_SIGMA_9_1 2
#define BLAKE2_SIGMA_9_2 8
#define BLAKE2_SIGMA_9_3 4
#define BLAKE2_SIGMA_9_4 7
#define BLAKE2_SIGMA_9_5 6
#define BLAKE2_SIGMA_9_6 1
#define BLAKE2_SIGMA_9_7 5
#define BLAKE2_SIGMA_9_8 15
#define BLAKE2_SIGMA_9_9 11
#define BLAKE2_SIGMA_9_10 9
#define BLAKE2_SIGMA_9_11 14
#define BLAKE2_SIGMA_9_12 3
#define BLAKE2_SIGMA_9_13 12
#define BLAKE2_SIGMA_9_14 13
#define BLAKE2_SIGMA_9_15 0

#define BLAKE2_SIGMA_10_0 0
#define BLAKE2_SIGMA_10_1 1
#define BLAKE2_SIGMA_10_2 2
#define BLAKE2_SIGMA_10_3 3
#define BLAKE2_SIGMA_10_4 4
#define BLAKE2_SIGMA_10_5 5
#define BLAKE2_SIGMA_10_6 6
#define BLAKE2_SIGMA_10_7 7
#define BLAKE2_SIGMA_10_8 8
#define BLAKE2_SIGMA_10_9 9
#define BLAKE2_SIGMA_10_10 10
#define BLAKE2_SIGMA_10_11 11
#define BLAKE2_SIGMA_10_12 12
#define BLAKE2_SIGMA_10_13 13
#define BLAKE2_SIGMA_10_14 14
#define BLAKE2_SIGMA_10_15 15

#define BLAKE2_SIGMA_11_0 14
#define BLAKE2_SIGMA_11_1 10
#define BLAKE2_SIGMA_11_2 4
#define BLAKE2_SIGMA_11_3 8
#define BLAKE2_SIGMA_11_4 9
#define BLAKE2_SIGMA_11_5 15
#define BLAKE2_SIGMA_11_6 13
#define BLAKE2_SIGMA_11_7 6
#define BLAKE2_SIGMA_11_8 1
#define BLAKE2_SIGMA_11_9 12
#define BLAKE2_SIGMA_11_10 0
#define BLAKE2_SIGMA_11_11 2
#define BLAKE2_SIGMA_11_12 11
#define BLAKE2_SIGMA_11_13 7
#define BLAKE2_SIGMA_11_14 5
#define BLAKE2_SIGMA_11_15 3

static FORCE_INLINE uint64_t rotr64(const uint64_t w, const unsigned c) {
	return (w >> c) | (w << (64 - c));
}

static FORCE_INLINE void blake2b_set_lastblock(blake2b_state* S) {
	S->f[0] = (uint64_t)-1;
}

static FORCE_INLINE void blake2b_increment_counter(blake2b_state* S,
	uint64_t inc) {
	S->t[0] += inc;
	S->t[1] += (S->t[0] < inc);
}

static FORCE_INLINE void blake2b_init0(blake2b_state* S) {
	memset(S, 0, sizeof(*S));
	memcpy(S->h, blake2b_IV, sizeof(S->h));
}

int hashx_blake2b_init_param(blake2b_state* S, const blake2b_param* P) {
	const unsigned char* p = (const unsigned char*)P;
	unsigned int i;

	if (NULL == P || NULL == S) {
		return -1;
	}

	blake2b_init0(S);
	/* IV XOR Parameter Block */
	for (i = 0; i < 8; ++i) {
		S->h[i] ^= load64(&p[i * sizeof(S->h[i])]);
	}
	S->outlen = P->digest_length;
	return 0;
}

#define SIGMA(r, k) BLAKE2_SIGMA_ ## r ## _ ## k

#define G(r, i, j, a, b, c, d)                                               \
    do {                                                                     \
        a = a + b + m[SIGMA(r, i)];                                          \
        d = rotr64(d ^ a, 32);                                               \
        c = c + d;                                                           \
        b = rotr64(b ^ c, 24);                                               \
        a = a + b + m[SIGMA(r, j)];                                          \
        d = rotr64(d ^ a, 16);                                               \
        c = c + d;                                                           \
        b = rotr64(b ^ c, 63);                                               \
    } while ((void)0, 0)

#define ROUND_INNER(r)                                                       \
    do {                                                                     \
        G(r,  0,  1, v[0], v[4], v[8], v[12]);                               \
        G(r,  2,  3, v[1], v[5], v[9], v[13]);                               \
        G(r,  4,  5, v[2], v[6], v[10], v[14]);                              \
        G(r,  6,  7, v[3], v[7], v[11], v[15]);                              \
        G(r,  8,  9, v[0], v[5], v[10], v[15]);                              \
        G(r, 10, 11, v[1], v[6], v[11], v[12]);                              \
        G(r, 12, 13, v[2], v[7], v[8], v[13]);                               \
        G(r, 14, 15, v[3], v[4], v[9], v[14]);                               \
    } while ((void)0, 0)

#define ROUND(r) ROUND_INNER(r)

static void blake2b_compress(blake2b_state* S, const uint8_t* block) {
	uint64_t m[16];
	uint64_t v[16];
	unsigned int i;

	for (i = 0; i < 16; ++i) {
		m[i] = load64(block + i * sizeof(m[i]));
	}

	for (i = 0; i < 8; ++i) {
		v[i] = S->h[i];
	}

	v[8] = blake2b_IV[0];
	v[9] = blake2b_IV[1];
	v[10] = blake2b_IV[2];
	v[11] = blake2b_IV[3];
	v[12] = blake2b_IV[4] ^ S->t[0];
	v[13] = blake2b_IV[5] ^ S->t[1];
	v[14] = blake2b_IV[6] ^ S->f[0];
	v[15] = blake2b_IV[7] ^ S->f[1];

	ROUND(0);
	ROUND(1);
	ROUND(2);
	ROUND(3);
	ROUND(4);
	ROUND(5);
	ROUND(6);
	ROUND(7);
	ROUND(8);
	ROUND(9);
	ROUND(10);
	ROUND(11);

	for (i = 0; i < 8; ++i) {
		S->h[i] = S->h[i] ^ v[i] ^ v[i + 8];
	}
}

static void blake2b_compress_4r(blake2b_state* S, const uint8_t* block) {
	uint64_t m[16];
	uint64_t v[16];
	unsigned int i;

	for (i = 0; i < 16; ++i) {
		m[i] = load64(block + i * sizeof(m[i]));
	}

	for (i = 0; i < 8; ++i) {
		v[i] = S->h[i];
	}

	v[8] = blake2b_IV[0];
	v[9] = blake2b_IV[1];
	v[10] = blake2b_IV[2];
	v[11] = blake2b_IV[3];
	v[12] = blake2b_IV[4] ^ S->t[0];
	v[13] = blake2b_IV[5] ^ S->t[1];
	v[14] = blake2b_IV[6] ^ S->f[0];
	v[15] = blake2b_IV[7] ^ S->f[1];

	ROUND(0);
	ROUND(1);
	ROUND(2);
	ROUND(3);

	for (i = 0; i < 8; ++i) {
		S->h[i] = S->h[i] ^ v[i] ^ v[i + 8];
	}
}

int hashx_blake2b_update(blake2b_state* S, const void* in, size_t inlen) {
	const uint8_t* pin = (const uint8_t*)in;

	if (inlen == 0) {
		return 0;
	}

	/* Sanity check */
	if (S == NULL || in == NULL) {
		return -1;
	}

	/* Is this a reused state? */
	if (S->f[0] != 0) {
		return -1;
	}

	if (S->buflen + inlen > BLAKE2B_BLOCKBYTES) {
		/* Complete current block */
		size_t left = S->buflen;
		size_t fill = BLAKE2B_BLOCKBYTES - left;
		memcpy(&S->buf[left], pin, fill);
		blake2b_increment_counter(S, BLAKE2B_BLOCKBYTES);
		blake2b_compress(S, S->buf);
		S->buflen = 0;
		inlen -= fill;
		pin += fill;
		/* Avoid buffer copies when possible */
		while (inlen > BLAKE2B_BLOCKBYTES) {
			blake2b_increment_counter(S, BLAKE2B_BLOCKBYTES);
			blake2b_compress(S, pin);
			inlen -= BLAKE2B_BLOCKBYTES;
			pin += BLAKE2B_BLOCKBYTES;
		}
	}
	memcpy(&S->buf[S->buflen], pin, inlen);
	S->buflen += (unsigned int)inlen;
	return 0;
}

int hashx_blake2b_final(blake2b_state* S, void* out, size_t outlen) {
	uint8_t buffer[BLAKE2B_OUTBYTES] = { 0 };
	unsigned int i;

	/* Sanity checks */
	if (S == NULL || out == NULL || outlen < S->outlen) {
		return -1;
	}

	/* Is this a reused state? */
	if (S->f[0] != 0) {
		return -1;
	}

	blake2b_increment_counter(S, S->buflen);
	blake2b_set_lastblock(S);
	memset(&S->buf[S->buflen], 0, BLAKE2B_BLOCKBYTES - S->buflen); /* Padding */
	blake2b_compress(S, S->buf);

	for (i = 0; i < 8; ++i) { /* Output full hash to temp buffer */
		store64(buffer + sizeof(S->h[i]) * i, S->h[i]);
	}

	memcpy(out, buffer, S->outlen);

	return 0;
}

/* 4-round version of Blake2b */
void hashx_blake2b_4r(const blake2b_param* params, const void* in,
	size_t inlen, void* out) {

	blake2b_state state;
	const uint8_t* p = (const uint8_t*)params;

	blake2b_init0(&state);
	/* IV XOR Parameter Block */
	for (unsigned i = 0; i < 8; ++i) {
		state.h[i] ^= load64(&p[i * sizeof(state.h[i])]);
	}
	//state.outlen = blake_params.digest_length;

	const uint8_t* pin = (const uint8_t*)in;

	while (inlen > BLAKE2B_BLOCKBYTES) {
		blake2b_increment_counter(&state, BLAKE2B_BLOCKBYTES);
		blake2b_compress_4r(&state, pin);
		inlen -= BLAKE2B_BLOCKBYTES;
		pin += BLAKE2B_BLOCKBYTES;
	}

	memcpy(state.buf, pin, inlen);
	blake2b_increment_counter(&state, inlen);
	blake2b_set_lastblock(&state);
	blake2b_compress_4r(&state, state.buf);

	/* Output hash */
	memcpy(out, state.h, sizeof(state.h));
}
