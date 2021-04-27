/*
 * Non-physical true random number generator based on timing jitter.
 *
 * Copyright Stephan Mueller <smueller@chronox.de>, 2014 - 2021
 *
 * Design
 * ======
 *
 * See documentation in doc/ folder.
 *
 * Interface
 * =========
 *
 * See documentation in jitterentropy(3) man page.
 *
 * License
 * =======
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU General Public License, in which case the provisions of the GPL2 are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "jitterentropy.h"

#define MAJVERSION 3 /* API / ABI incompatible changes, functional changes that
		      * require consumer to be updated (as long as this number
		      * is zero, the API is not considered stable and can
		      * change without a bump of the major version) */
#define MINVERSION 0 /* API compatible, ABI may change, functional
		      * enhancements only, consumer can be left unchanged if
		      * enhancements are not considered */
#define PATCHLEVEL 2 /* API / ABI compatible, no functional changes, no
		      * enhancements, bug fixes only */

/***************************************************************************
 * Jitter RNG Static Definitions
 *
 * None of the following should be altered
 ***************************************************************************/

#ifdef __OPTIMIZE__
 #error "The CPU Jitter random number generator must not be compiled with optimizations. See documentation. Use the compiler switch -O0 for compiling jitterentropy.c."
#endif

/*
 * JENT_POWERUP_TESTLOOPCOUNT needs some loops to identify edge
 * systems. 100 is definitely too little.
 *
 * SP800-90B requires at least 1024 initial test cycles.
 */
#define JENT_POWERUP_TESTLOOPCOUNT 1024

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * jent_version() - Return machine-usable version number of jent library
 *
 * The function returns a version number that is monotonic increasing
 * for newer versions. The version numbers are multiples of 100. For example,
 * version 1.2.3 is converted to 1020300 -- the last two digits are reserved
 * for future use.
 *
 * The result of this function can be used in comparing the version number
 * in a calling program if version-specific calls need to be make.
 *
 * @return Version number of jitterentropy library
 */
JENT_PRIVATE_STATIC
unsigned int jent_version(void)
{
	unsigned int version = 0;

	version =  MAJVERSION * 1000000;
	version += MINVERSION * 10000;
	version += PATCHLEVEL * 100;

	return version;
}

/***************************************************************************
 * Adaptive Proportion Test
 *
 * This test complies with SP800-90B section 4.4.2.
 ***************************************************************************/

/**
 * Reset the APT counter
 *
 * @ec [in] Reference to entropy collector
 */
static void jent_apt_reset(struct rand_data *ec, uint64_t current_delta)
{
	/* Reset APT counter */
	ec->apt_count = 0;
	ec->apt_base = current_delta;
	ec->apt_observations = 0;
}

/**
 * Insert a new entropy event into APT
 *
 * @ec [in] Reference to entropy collector
 * @current_delta [in] Current time delta
 */
static void jent_apt_insert(struct rand_data *ec, uint64_t current_delta)
{
	/* Initialize the base reference */
	if (!ec->apt_base_set) {
		ec->apt_base = current_delta;
		ec->apt_base_set = 1;
		return;
	}

	if (current_delta == ec->apt_base) {
		ec->apt_count++;

		if (ec->apt_count >= JENT_APT_CUTOFF)
			ec->health_failure = 1;
	}

	ec->apt_observations++;

	if (ec->apt_observations >= JENT_APT_WINDOW_SIZE)
		jent_apt_reset(ec, current_delta);
}

/***************************************************************************
 * Stuck Test and its use as Repetition Count Test
 *
 * The Jitter RNG uses an enhanced version of the Repetition Count Test
 * (RCT) specified in SP800-90B section 4.4.1. Instead of counting identical
 * back-to-back values, the input to the RCT is the counting of the stuck
 * values during the generation of one Jitter RNG output block.
 *
 * The RCT is applied with an alpha of 2^{-30} compliant to FIPS 140-2 IG 9.8.
 *
 * During the counting operation, the Jitter RNG always calculates the RCT
 * cut-off value of C. If that value exceeds the allowed cut-off value,
 * the Jitter RNG output block will be calculated completely but discarded at
 * the end. The caller of the Jitter RNG is informed with an error code.
 ***************************************************************************/

/**
 * Repetition Count Test as defined in SP800-90B section 4.4.1
 *
 * @ec [in] Reference to entropy collector
 * @stuck [in] Indicator whether the value is stuck
 */
static void jent_rct_insert(struct rand_data *ec, int stuck)
{
	/*
	 * If we have a count less than zero, a previous RCT round identified
	 * a failure. We will not overwrite it.
	 */
	if (ec->rct_count < 0)
		return;

	if (stuck) {
		ec->rct_count++;

		/*
		 * The cutoff value is based on the following consideration:
		 * alpha = 2^-30 as recommended in FIPS 140-2 IG 9.8.
		 * In addition, we require an entropy value H of 1/OSR as this
		 * is the minimum entropy required to provide full entropy.
		 * Note, we collect 64 * OSR deltas for inserting them into
		 * the entropy pool which should then have (close to) 64 bits
		 * of entropy.
		 *
		 * Note, ec->rct_count (which equals to value B in the pseudo
		 * code of SP800-90B section 4.4.1) starts with zero. Hence
		 * we need to subtract one from the cutoff value as calculated
		 * following SP800-90B.
		 */
		if ((unsigned int)ec->rct_count >= (31 * ec->osr)) {
			ec->rct_count = -1;
			ec->health_failure = 1;
		}
	} else {
		ec->rct_count = 0;
	}
}

/**
 * Is there an RCT health test failure?
 *
 * @ec [in] Reference to entropy collector
 *
 * @return
 * 	0 No health test failure
 * 	1 Permanent health test failure
 */
static int jent_rct_failure(struct rand_data *ec)
{
	if (ec->rct_count < 0)
		return 1;
	return 0;
}

static inline uint64_t jent_delta(uint64_t prev, uint64_t next)
{
	return (next - prev);
}

/**
 * Stuck test by checking the:
 * 	1st derivative of the jitter measurement (time delta)
 * 	2nd derivative of the jitter measurement (delta of time deltas)
 * 	3rd derivative of the jitter measurement (delta of delta of time deltas)
 *
 * All values must always be non-zero.
 *
 * @ec [in] Reference to entropy collector
 * @current_delta [in] Jitter time delta
 *
 * @return
 * 	0 jitter measurement not stuck (good bit)
 * 	1 jitter measurement stuck (reject bit)
 */
static unsigned int jent_stuck(struct rand_data *ec, uint64_t current_delta)
{
	uint64_t delta2 = jent_delta(ec->last_delta, current_delta);
	uint64_t delta3 = jent_delta(ec->last_delta2, delta2);

	ec->last_delta = current_delta;
	ec->last_delta2 = delta2;

	/*
	 * Insert the result of the comparison of two back-to-back time
	 * deltas.
	 */
	jent_apt_insert(ec, current_delta);

	if (!current_delta || !delta2 || !delta3) {
		/* RCT with a stuck bit */
		jent_rct_insert(ec, 1);
		return 1;
	}

	/* RCT with a non-stuck bit */
	jent_rct_insert(ec, 0);

	return 0;
}

/**
 * Report any health test failures
 *
 * @ec [in] Reference to entropy collector
 *
 * @return
 * 	0 No health test failure
 * 	1 Permanent health test failure
 */
static int jent_health_failure(struct rand_data *ec)
{
	/* Test is only enabled in FIPS mode */
	if (!ec->fips_enabled)
		return 0;

	return ec->health_failure;
}

/***************************************************************************
 * Message Digest Implementation
 ***************************************************************************/
#define SHA3_SIZE_BLOCK(bits)		((1600 - 2 * bits) >> 3)
#define SHA3_256_SIZE_BLOCK		SHA3_SIZE_BLOCK(SHA3_256_SIZE_DIGEST_BITS)
#define SHA3_MAX_SIZE_BLOCK		SHA3_256_SIZE_BLOCK

struct sha_ctx {
	uint64_t state[25];
	size_t msg_len;
	unsigned int r;
	unsigned int rword;
	unsigned int digestsize;
	uint8_t partial[SHA3_MAX_SIZE_BLOCK];
};

#define aligned(val)	__attribute__((aligned(val)))
#define ALIGNED_BUFFER(name, size, type)				       \
	type name[(size + sizeof(type)-1) / sizeof(type)] aligned(sizeof(type));

/* CTX size allows any hash type up to SHA3-224 */
#define SHA_MAX_CTX_SIZE	368
#define HASH_CTX_ON_STACK(name)						       \
	ALIGNED_BUFFER(name ## _ctx_buf, SHA_MAX_CTX_SIZE, uint64_t)	       \
	struct sha_ctx *name = (struct sha_ctx *) name ## _ctx_buf

/*
 * Conversion of Little-Endian representations in byte streams - the data
 * representation in the integer values is the host representation.
 */
static inline uint32_t ptr_to_le32(const uint8_t *p)
{
	return (uint32_t)p[0]       | (uint32_t)p[1] << 8 |
	       (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static inline uint64_t ptr_to_le64(const uint8_t *p)
{
	return (uint64_t)ptr_to_le32(p) | (uint64_t)ptr_to_le32(p + 4) << 32;
}

static inline void le32_to_ptr(uint8_t *p, const uint32_t value)
{
	p[0] = (uint8_t)(value);
	p[1] = (uint8_t)(value >> 8);
	p[2] = (uint8_t)(value >> 16);
	p[3] = (uint8_t)(value >> 24);
}

static inline void le64_to_ptr(uint8_t *p, const uint64_t value)
{
	le32_to_ptr(p + 4, (uint32_t)(value >> 32));
	le32_to_ptr(p,     (uint32_t)(value));
}

/*********************************** Keccak ***********************************/
/* state[x + y*5] */
#define A(x, y) (x + 5 * y)

static inline void keccakp_theta(uint64_t s[25])
{
	uint64_t C[5], D[5];

	/* Step 1 */
	C[0] = s[A(0, 0)] ^ s[A(0, 1)] ^ s[A(0, 2)] ^ s[A(0, 3)] ^ s[A(0, 4)];
	C[1] = s[A(1, 0)] ^ s[A(1, 1)] ^ s[A(1, 2)] ^ s[A(1, 3)] ^ s[A(1, 4)];
	C[2] = s[A(2, 0)] ^ s[A(2, 1)] ^ s[A(2, 2)] ^ s[A(2, 3)] ^ s[A(2, 4)];
	C[3] = s[A(3, 0)] ^ s[A(3, 1)] ^ s[A(3, 2)] ^ s[A(3, 3)] ^ s[A(3, 4)];
	C[4] = s[A(4, 0)] ^ s[A(4, 1)] ^ s[A(4, 2)] ^ s[A(4, 3)] ^ s[A(4, 4)];

	/* Step 2 */
	D[0] = C[4] ^ rol64(C[1], 1);
	D[1] = C[0] ^ rol64(C[2], 1);
	D[2] = C[1] ^ rol64(C[3], 1);
	D[3] = C[2] ^ rol64(C[4], 1);
	D[4] = C[3] ^ rol64(C[0], 1);

	/* Step 3 */
	s[A(0, 0)] ^= D[0];
	s[A(1, 0)] ^= D[1];
	s[A(2, 0)] ^= D[2];
	s[A(3, 0)] ^= D[3];
	s[A(4, 0)] ^= D[4];

	s[A(0, 1)] ^= D[0];
	s[A(1, 1)] ^= D[1];
	s[A(2, 1)] ^= D[2];
	s[A(3, 1)] ^= D[3];
	s[A(4, 1)] ^= D[4];

	s[A(0, 2)] ^= D[0];
	s[A(1, 2)] ^= D[1];
	s[A(2, 2)] ^= D[2];
	s[A(3, 2)] ^= D[3];
	s[A(4, 2)] ^= D[4];

	s[A(0, 3)] ^= D[0];
	s[A(1, 3)] ^= D[1];
	s[A(2, 3)] ^= D[2];
	s[A(3, 3)] ^= D[3];
	s[A(4, 3)] ^= D[4];

	s[A(0, 4)] ^= D[0];
	s[A(1, 4)] ^= D[1];
	s[A(2, 4)] ^= D[2];
	s[A(3, 4)] ^= D[3];
	s[A(4, 4)] ^= D[4];
}

static inline void keccakp_rho(uint64_t s[25])
{
	/* Step 1 */
	/* s[A(0, 0)] = s[A(0, 0)]; */

#define RHO_ROL(t)	(((t + 1) * (t + 2) / 2) % 64)
	/* Step 3 */
	s[A(1, 0)] = rol64(s[A(1, 0)], RHO_ROL(0));
	s[A(0, 2)] = rol64(s[A(0, 2)], RHO_ROL(1));
	s[A(2, 1)] = rol64(s[A(2, 1)], RHO_ROL(2));
	s[A(1, 2)] = rol64(s[A(1, 2)], RHO_ROL(3));
	s[A(2, 3)] = rol64(s[A(2, 3)], RHO_ROL(4));
	s[A(3, 3)] = rol64(s[A(3, 3)], RHO_ROL(5));
	s[A(3, 0)] = rol64(s[A(3, 0)], RHO_ROL(6));
	s[A(0, 1)] = rol64(s[A(0, 1)], RHO_ROL(7));
	s[A(1, 3)] = rol64(s[A(1, 3)], RHO_ROL(8));
	s[A(3, 1)] = rol64(s[A(3, 1)], RHO_ROL(9));
	s[A(1, 4)] = rol64(s[A(1, 4)], RHO_ROL(10));
	s[A(4, 4)] = rol64(s[A(4, 4)], RHO_ROL(11));
	s[A(4, 0)] = rol64(s[A(4, 0)], RHO_ROL(12));
	s[A(0, 3)] = rol64(s[A(0, 3)], RHO_ROL(13));
	s[A(3, 4)] = rol64(s[A(3, 4)], RHO_ROL(14));
	s[A(4, 3)] = rol64(s[A(4, 3)], RHO_ROL(15));
	s[A(3, 2)] = rol64(s[A(3, 2)], RHO_ROL(16));
	s[A(2, 2)] = rol64(s[A(2, 2)], RHO_ROL(17));
	s[A(2, 0)] = rol64(s[A(2, 0)], RHO_ROL(18));
	s[A(0, 4)] = rol64(s[A(0, 4)], RHO_ROL(19));
	s[A(4, 2)] = rol64(s[A(4, 2)], RHO_ROL(20));
	s[A(2, 4)] = rol64(s[A(2, 4)], RHO_ROL(21));
	s[A(4, 1)] = rol64(s[A(4, 1)], RHO_ROL(22));
	s[A(1, 1)] = rol64(s[A(1, 1)], RHO_ROL(23));
}

static inline void keccakp_pi(uint64_t s[25])
{
	uint64_t t = s[A(4, 4)];

	/* Step 1 */
	/* s[A(0, 0)] = s[A(0, 0)]; */
	s[A(4, 4)] = s[A(1, 4)];
	s[A(1, 4)] = s[A(3, 1)];
	s[A(3, 1)] = s[A(1, 3)];
	s[A(1, 3)] = s[A(0, 1)];
	s[A(0, 1)] = s[A(3, 0)];
	s[A(3, 0)] = s[A(3, 3)];
	s[A(3, 3)] = s[A(2, 3)];
	s[A(2, 3)] = s[A(1, 2)];
	s[A(1, 2)] = s[A(2, 1)];
	s[A(2, 1)] = s[A(0, 2)];
	s[A(0, 2)] = s[A(1, 0)];
	s[A(1, 0)] = s[A(1, 1)];
	s[A(1, 1)] = s[A(4, 1)];
	s[A(4, 1)] = s[A(2, 4)];
	s[A(2, 4)] = s[A(4, 2)];
	s[A(4, 2)] = s[A(0, 4)];
	s[A(0, 4)] = s[A(2, 0)];
	s[A(2, 0)] = s[A(2, 2)];
	s[A(2, 2)] = s[A(3, 2)];
	s[A(3, 2)] = s[A(4, 3)];
	s[A(4, 3)] = s[A(3, 4)];
	s[A(3, 4)] = s[A(0, 3)];
	s[A(0, 3)] = s[A(4, 0)];
	s[A(4, 0)] = t;
}

static inline void keccakp_chi(uint64_t s[25])
{
	uint64_t t0[5], t1[5];

	t0[0] = s[A(0, 0)];
	t0[1] = s[A(0, 1)];
	t0[2] = s[A(0, 2)];
	t0[3] = s[A(0, 3)];
	t0[4] = s[A(0, 4)];

	t1[0] = s[A(1, 0)];
	t1[1] = s[A(1, 1)];
	t1[2] = s[A(1, 2)];
	t1[3] = s[A(1, 3)];
	t1[4] = s[A(1, 4)];

	s[A(0, 0)] ^= ~s[A(1, 0)] & s[A(2, 0)];
	s[A(0, 1)] ^= ~s[A(1, 1)] & s[A(2, 1)];
	s[A(0, 2)] ^= ~s[A(1, 2)] & s[A(2, 2)];
	s[A(0, 3)] ^= ~s[A(1, 3)] & s[A(2, 3)];
	s[A(0, 4)] ^= ~s[A(1, 4)] & s[A(2, 4)];

	s[A(1, 0)] ^= ~s[A(2, 0)] & s[A(3, 0)];
	s[A(1, 1)] ^= ~s[A(2, 1)] & s[A(3, 1)];
	s[A(1, 2)] ^= ~s[A(2, 2)] & s[A(3, 2)];
	s[A(1, 3)] ^= ~s[A(2, 3)] & s[A(3, 3)];
	s[A(1, 4)] ^= ~s[A(2, 4)] & s[A(3, 4)];

	s[A(2, 0)] ^= ~s[A(3, 0)] & s[A(4, 0)];
	s[A(2, 1)] ^= ~s[A(3, 1)] & s[A(4, 1)];
	s[A(2, 2)] ^= ~s[A(3, 2)] & s[A(4, 2)];
	s[A(2, 3)] ^= ~s[A(3, 3)] & s[A(4, 3)];
	s[A(2, 4)] ^= ~s[A(3, 4)] & s[A(4, 4)];

	s[A(3, 0)] ^= ~s[A(4, 0)] & t0[0];
	s[A(3, 1)] ^= ~s[A(4, 1)] & t0[1];
	s[A(3, 2)] ^= ~s[A(4, 2)] & t0[2];
	s[A(3, 3)] ^= ~s[A(4, 3)] & t0[3];
	s[A(3, 4)] ^= ~s[A(4, 4)] & t0[4];

	s[A(4, 0)] ^= ~t0[0] & t1[0];
	s[A(4, 1)] ^= ~t0[1] & t1[1];
	s[A(4, 2)] ^= ~t0[2] & t1[2];
	s[A(4, 3)] ^= ~t0[3] & t1[3];
	s[A(4, 4)] ^= ~t0[4] & t1[4];
}

static const uint64_t keccakp_iota_vals[] = {
	0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
	0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
	0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
	0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
	0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
	0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
	0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
	0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

static inline void keccakp_iota(uint64_t s[25], unsigned int round)
{
	s[0] ^= keccakp_iota_vals[round];
}

static inline void keccakp_1600(uint64_t s[25])
{
	unsigned int round;

	for (round = 0; round < 24; round++) {
		keccakp_theta(s);
		keccakp_rho(s);
		keccakp_pi(s);
		keccakp_chi(s);
		keccakp_iota(s, round);
	}
}

/*********************************** SHA-3 ************************************/

static inline void sha3_init(struct sha_ctx *ctx)
{
	unsigned int i;

	for (i = 0; i < 25; i++)
		ctx->state[i] = 0;
	ctx->msg_len = 0;
}

static void sha3_256_init(struct sha_ctx *ctx)
{
	sha3_init(ctx);
	ctx->r = SHA3_256_SIZE_BLOCK;
	ctx->rword = SHA3_256_SIZE_BLOCK / sizeof(uint64_t);
	ctx->digestsize = SHA3_256_SIZE_DIGEST;
}

static inline void sha3_fill_state(struct sha_ctx *ctx, const uint8_t *in)
{
	unsigned int i;

	for (i = 0; i < ctx->rword; i++) {
		ctx->state[i]  ^= ptr_to_le64(in);
		in += 8;
	}
}

static void sha3_update(struct sha_ctx *ctx, const uint8_t *in, size_t inlen)
{
	size_t partial = ctx->msg_len % ctx->r;

	ctx->msg_len += inlen;

	/* Sponge absorbing phase */

	/* Check if we have a partial block stored */
	if (partial) {
		size_t todo = ctx->r - partial;

		/*
		 * If the provided data is small enough to fit in the partial
		 * buffer, copy it and leave it unprocessed.
		 */
		if (inlen < todo) {
			memcpy(ctx->partial + partial, in, inlen);
			return;
		}

		/*
		 * The input data is large enough to fill the entire partial
		 * block buffer. Thus, we fill it and transform it.
		 */
		memcpy(ctx->partial + partial, in, todo);
		inlen -= todo;
		in += todo;

		sha3_fill_state(ctx, ctx->partial);
		keccakp_1600(ctx->state);
	}

	/* Perform a transformation of full block-size messages */
	for (; inlen >= ctx->r; inlen -= ctx->r, in += ctx->r) {
		sha3_fill_state(ctx, in);
		keccakp_1600(ctx->state);
	}

	/* If we have data left, copy it into the partial block buffer */
	memcpy(ctx->partial, in, inlen);
}

static void sha3_final(struct sha_ctx *ctx, uint8_t *digest)
{
	size_t partial = ctx->msg_len % ctx->r;
	unsigned int i;

	/* Final round in sponge absorbing phase */

	/* Fill the unused part of the partial buffer with zeros */
	memset(ctx->partial + partial, 0, ctx->r - partial);

	/*
	 * Add the leading and trailing bit as well as the 01 bits for the
	 * SHA-3 suffix.
	 */
	ctx->partial[partial] = 0x06;
	ctx->partial[ctx->r - 1] |= 0x80;

	/* Final transformation */
	sha3_fill_state(ctx, ctx->partial);
	keccakp_1600(ctx->state);

	/*
	 * Sponge squeeze phase - the digest size is always smaller as the
	 * state size r which implies we only have one squeeze round.
	 */
	for (i = 0; i < ctx->digestsize / 8; i++, digest += 8)
		le64_to_ptr(digest, ctx->state[i]);

	/* Add remaining 4 bytes if we use SHA3-224 */
	if (ctx->digestsize % 8)
		le32_to_ptr(digest, (uint32_t)(ctx->state[i]));

	memset(ctx->partial, 0, ctx->r);
	sha3_init(ctx);
}

static int sha3_tester(void)
{
	HASH_CTX_ON_STACK(ctx);
	static const uint8_t msg_256[] = { 0x5E, 0x5E, 0xD6 };
	static const uint8_t exp_256[] = { 0xF1, 0x6E, 0x66, 0xC0, 0x43, 0x72,
					   0xB4, 0xA3, 0xE1, 0xE3, 0x2E, 0x07,
					   0xC4, 0x1C, 0x03, 0x40, 0x8A, 0xD5,
					   0x43, 0x86, 0x8C, 0xC4, 0x0E, 0xC5,
					   0x5E, 0x00, 0xBB, 0xBB, 0xBD, 0xF5,
					   0x91, 0x1E };
	uint8_t act[SHA3_256_SIZE_DIGEST] = { 0 };
	unsigned int i;

	sha3_256_init(ctx);
	sha3_update(ctx, msg_256, 3);
	sha3_final(ctx, act);

	for (i = 0; i < SHA3_256_SIZE_DIGEST; i++) {
		if (exp_256[i] != act[i])
			return 1;
	}

	return 0;
}

#ifdef JENT_CONF_ENABLE_INTERNAL_TIMER

/***************************************************************************
 * Timer-less timer replacement
 *
 * If there is no high-resolution hardware timer available, we create one
 * ourselves. This logic is only used when the initialization identifies
 * that no suitable time source is available.
 ***************************************************************************/

static int jent_force_internal_timer = 0;

/**
 * Timer-replacement loop
 *
 * @brief The measurement loop triggers the read of the value from the
 * counter function. It conceptually acts as the low resolution
 * sampleS timer from a ring oscillator.
 */
static void *jent_notime_sample_timer(void *arg)
{
	struct rand_data *ec = (struct rand_data *)arg;

	ec->notime_timer = 0;

	while (1) {
		if (ec->notime_interrupt)
			return NULL;

		ec->notime_timer++;
	}

	return NULL;
}

/*
 * Enable the clock: spawn a new thread that holds a counter.
 *
 * Note, although creating a thread is expensive, we do that every time a
 * caller wants entropy from us and terminate the thread afterwards. This
 * is to ensure an attacker cannot easily identify the ticking thread.
 */
static inline int jent_notime_settick(struct rand_data *ec)
{
	int ret;

	if (!ec->enable_notime)
		return 0;

	ret = -pthread_attr_init(&ec->notime_pthread_attr);
	if (ret)
		return ret;

	ec->notime_interrupt = 0;
	ec->notime_prev_timer = 0;
	ec->notime_timer = 0;

	return -pthread_create(&ec->notime_thread_id,
			       &ec->notime_pthread_attr,
			       jent_notime_sample_timer, ec);
}

static inline void jent_notime_unsettick(struct rand_data *ec)
{
	if (!ec->enable_notime)
		return;

	ec->notime_interrupt = 1;
	pthread_join(ec->notime_thread_id, NULL);
	pthread_attr_destroy(&ec->notime_pthread_attr);
}

static inline void jent_get_nstime_internal(struct rand_data *ec, uint64_t *out)
{
	if (ec->enable_notime) {
		/*
		 * Allow the counting thread to be initialized and guarantee
		 * that it ticked since last time we looked.
		 *
		 * Note, we do not use an atomic operation here for reading
		 * jent_notime_timer since if this integer is garbled, it even
		 * adds to entropy. But on most architectures, read/write
		 * of an uint64_t should be atomic anyway.
		 */
		while (ec->notime_timer == ec->notime_prev_timer)
			;

		ec->notime_prev_timer = ec->notime_timer;
		*out = ec->notime_prev_timer;
	} else {
		jent_get_nstime(out);
	}
}

static int jent_time_entropy_init(unsigned int enable_notime);
static int jent_notime_enable(struct rand_data *ec, unsigned int flags)
{
	/* Use internal timer */
	if (jent_force_internal_timer || (flags & JENT_FORCE_INTERNAL_TIMER)) {
		/* Self test not run yet */
		if (!jent_force_internal_timer && jent_time_entropy_init(1))
			return EHEALTH;

		ec->enable_notime = 1;
	}

	return 0;
}

#else /* JENT_CONF_ENABLE_INTERNAL_TIMER */

static inline void jent_get_nstime_internal(struct rand_data *ec, uint64_t *out)
{
	(void)ec;
	jent_get_nstime(out);
}

static inline int jent_notime_enable(struct rand_data *ec, unsigned int flags)
{
	(void)ec;

	/* If we force the timer-less noise source, we return an error */
	if (flags & JENT_FORCE_INTERNAL_TIMER)
		return EHEALTH;

	return 0;
}

static inline int jent_notime_settick(struct rand_data *ec)
{
	(void)ec;
	return 0;
}

static inline void jent_notime_unsettick(struct rand_data *ec) { (void)ec; }

#endif /* JENT_CONF_ENABLE_INTERNAL_TIMER */

/***************************************************************************
 * Noise sources
 ***************************************************************************/

/**
 * Update of the loop count used for the next round of
 * an entropy collection.
 *
 * @ec [in] entropy collector struct -- may be NULL
 * @bits [in] is the number of low bits of the timer to consider
 * @min [in] is the number of bits we shift the timer value to the right at
 *	     the end to make sure we have a guaranteed minimum value
 *
 * @return Newly calculated loop counter
 */
static uint64_t jent_loop_shuffle(struct rand_data *ec,
				  unsigned int bits, unsigned int min)
{
#ifdef JENT_CONF_DISABLE_LOOP_SHUFFLE

	(void)ec;
	(void)bits;

	return (1<<min);

#else /* JENT_CONF_DISABLE_LOOP_SHUFFLE */

	uint64_t time = 0;
	uint64_t shuffle = 0;
	unsigned int i = 0;
	unsigned int mask = (1U<<bits) - 1;

	/*
	 * Mix the current state of the random number into the shuffle
	 * calculation to balance that shuffle a bit more.
	 */
	if (ec) {
		jent_get_nstime_internal(ec, &time);
		time ^= ec->data[0];
	}

	/*
	 * We fold the time value as much as possible to ensure that as many
	 * bits of the time stamp are included as possible.
	 */
	for (i = 0; ((DATA_SIZE_BITS + bits - 1) / bits) > i; i++) {
		shuffle ^= time & mask;
		time = time >> bits;
	}

	/*
	 * We add a lower boundary value to ensure we have a minimum
	 * RNG loop count.
	 */
	return (shuffle + (1<<min));

#endif /* JENT_CONF_DISABLE_LOOP_SHUFFLE */
}

/**
 * CPU Jitter noise source -- this is the noise source based on the CPU
 * 			      execution time jitter
 *
 * This function injects the individual bits of the time value into the
 * entropy pool using a hash.
 *
 * @ec [in] entropy collector struct -- may be NULL
 * @time [in] time stamp to be injected
 * @loop_cnt [in] if a value not equal to 0 is set, use the given value as
 *		  number of loops to perform the hash operation
 * @stuck [in] Is the time stamp identified as stuck?
 *
 * Output:
 * updated hash context
 */
static void jent_hash_time(struct rand_data *ec, uint64_t time,
			   uint64_t loop_cnt, unsigned int stuck)
{
	HASH_CTX_ON_STACK(ctx);
	uint8_t itermediary[SHA3_256_SIZE_DIGEST];
	uint64_t j = 0;
#define MAX_HASH_LOOP 3
#define MIN_HASH_LOOP 0
	uint64_t hash_loop_cnt =
		jent_loop_shuffle(ec, MAX_HASH_LOOP, MIN_HASH_LOOP);

	sha3_256_init(ctx);

	/*
	 * testing purposes -- allow test app to set the counter, not
	 * needed during runtime
	 */
	if (loop_cnt)
		hash_loop_cnt = loop_cnt;

	/*
	 * This loop basically slows down the SHA-3 operation depending
	 * on the hash_loop_cnt. Each iteration of the loop generates the
	 * same result.
	 */
	for (j = 0; j < hash_loop_cnt; j++) {
		sha3_update(ctx, ec->data, SHA3_256_SIZE_DIGEST);
		sha3_update(ctx, (uint8_t *)&time, sizeof(uint64_t));
		sha3_update(ctx, (uint8_t *)&j, sizeof(uint64_t));

		/*
		 * If the time stamp is stuck, do not finally insert the value
		 * into the entropy pool. Although this operation should not do
		 * any harm even when the time stamp has no entropy, SP800-90B
		 * requires that any conditioning operation to have an identical
		 * amount of input data according to section 3.1.5.
		 */

		/*
		 * The sha3_final operations re-initialize the context for the
		 * next loop iteration.
		 */
		if (stuck || (j < hash_loop_cnt - 1))
			sha3_final(ctx, itermediary);
		else
			sha3_final(ctx, ec->data);
	}

	jent_memset_secure(ctx, SHA_MAX_CTX_SIZE);
	jent_memset_secure(itermediary, sizeof(itermediary));
}

/**
 * Memory Access noise source -- this is a noise source based on variations in
 * 				 memory access times
 *
 * This function performs memory accesses which will add to the timing
 * variations due to an unknown amount of CPU wait states that need to be
 * added when accessing memory. The memory size should be larger than the L1
 * caches as outlined in the documentation and the associated testing.
 *
 * The L1 cache has a very high bandwidth, albeit its access rate is  usually
 * slower than accessing CPU registers. Therefore, L1 accesses only add minimal
 * variations as the CPU has hardly to wait. Starting with L2, significant
 * variations are added because L2 typically does not belong to the CPU any more
 * and therefore a wider range of CPU wait states is necessary for accesses.
 * L3 and real memory accesses have even a wider range of wait states. However,
 * to reliably access either L3 or memory, the ec->mem memory must be quite
 * large which is usually not desirable.
 *
 * @ec [in] Reference to the entropy collector with the memory access data -- if
 *	    the reference to the memory block to be accessed is NULL, this noise
 *	    source is disabled
 * @loop_cnt [in] if a value not equal to 0 is set, use the given value as
 *		  number of loops to perform the hash operation
 */
static void jent_memaccess(struct rand_data *ec, uint64_t loop_cnt)
{
	unsigned int wrap = 0;
	uint64_t i = 0;
#define MAX_ACC_LOOP_BIT 7
#define MIN_ACC_LOOP_BIT 0
	uint64_t acc_loop_cnt =
		jent_loop_shuffle(ec, MAX_ACC_LOOP_BIT, MIN_ACC_LOOP_BIT);

	if (NULL == ec || NULL == ec->mem)
		return;
	wrap = ec->memblocksize * ec->memblocks;

	/*
	 * testing purposes -- allow test app to set the counter, not
	 * needed during runtime
	 */
	if (loop_cnt)
		acc_loop_cnt = loop_cnt;
	for (i = 0; i < (ec->memaccessloops + acc_loop_cnt); i++) {
		unsigned char *tmpval = ec->mem + ec->memlocation;
		/*
		 * memory access: just add 1 to one byte,
		 * wrap at 255 -- memory access implies read
		 * from and write to memory location
		 */
		*tmpval = (unsigned char)((*tmpval + 1) & 0xff);
		/*
		 * Addition of memblocksize - 1 to pointer
		 * with wrap around logic to ensure that every
		 * memory location is hit evenly
		 */
		ec->memlocation = ec->memlocation + ec->memblocksize - 1;
		ec->memlocation = ec->memlocation % wrap;
	}
}

/***************************************************************************
 * Start of entropy processing logic
 ***************************************************************************/

/**
 * This is the heart of the entropy generation: calculate time deltas and
 * use the CPU jitter in the time deltas. The jitter is injected into the
 * entropy pool.
 *
 * WARNING: ensure that ->prev_time is primed before using the output
 * 	    of this function! This can be done by calling this function
 * 	    and not using its result.
 *
 * @ec [in] Reference to entropy collector
 * @loop_cnt [in] see jent_hash_time
 * @ret_current_delta [out] Test interface: return time delta - may be NULL
 *
 * @return: result of stuck test
 */
static unsigned int jent_measure_jitter(struct rand_data *ec,
					uint64_t loop_cnt,
					uint64_t *ret_current_delta)
{
	uint64_t time = 0;
	uint64_t current_delta = 0;
	unsigned int stuck;

	/* Invoke one noise source before time measurement to add variations */
	jent_memaccess(ec, loop_cnt);

	/*
	 * Get time stamp and calculate time delta to previous
	 * invocation to measure the timing variations
	 */
	jent_get_nstime_internal(ec, &time);
	current_delta = jent_delta(ec->prev_time, time);
	ec->prev_time = time;

	/* Check whether we have a stuck measurement. */
	stuck = jent_stuck(ec, current_delta);

	/* Now call the next noise sources which also injects the data */
	jent_hash_time(ec, current_delta, loop_cnt, stuck);

	/* return the raw entropy value */
	if (ret_current_delta)
		*ret_current_delta = current_delta;

	return stuck;
}

/**
 * Generator of one 256 bit random number
 * Function fills rand_data->data
 *
 * @ec [in] Reference to entropy collector
 */
static void jent_random_data(struct rand_data *ec)
{
	unsigned int k = 0;

	/* priming of the ->prev_time value */
	jent_measure_jitter(ec, 0, NULL);

	while (1) {
		/* If a stuck measurement is received, repeat measurement */
		if (jent_measure_jitter(ec, 0, NULL))
			continue;

		/*
		 * We multiply the loop value with ->osr to obtain the
		 * oversampling rate requested by the caller
		 */
		if (++k >= (DATA_SIZE_BITS * ec->osr))
			break;
	}
}

/***************************************************************************
 * Random Number Generation
 ***************************************************************************/

/**
 * Entry function: Obtain entropy for the caller.
 *
 * This function invokes the entropy gathering logic as often to generate
 * as many bytes as requested by the caller. The entropy gathering logic
 * creates 64 bit per invocation.
 *
 * This function truncates the last 64 bit entropy value output to the exact
 * size specified by the caller.
 *
 * @ec [in] Reference to entropy collector
 * @data [out] pointer to buffer for storing random data -- buffer must
 *	       already exist
 * @len [in] size of the buffer, specifying also the requested number of random
 *	     in bytes
 *
 * @return number of bytes returned when request is fulfilled or an error
 *
 * The following error codes can occur:
 *	-1	entropy_collector is NULL
 *	-2	RCT failed
 *	-3	APT test failed
 *	-4	The timer cannot be initialized
 */
JENT_PRIVATE_STATIC
ssize_t jent_read_entropy(struct rand_data *ec, char *data, size_t len)
{
	char *p = data;
	size_t orig_len = len;
	int ret = 0;

	if (NULL == ec)
		return -1;

	if (jent_notime_settick(ec))
		return -4;

	while (len > 0) {
		size_t tocopy;

		jent_random_data(ec);

		if (jent_health_failure(ec)) {
			if (jent_rct_failure(ec))
				ret = -2;
			else
				ret = -3;

			goto err;
		}

		if ((DATA_SIZE_BITS / 8) < len)
			tocopy = (DATA_SIZE_BITS / 8);
		else
			tocopy = len;
		memcpy(p, &ec->data, tocopy);

		len -= tocopy;
		p += tocopy;
	}

	/*
	 * To be on the safe side, we generate one more round of entropy
	 * which we do not give out to the caller. That round shall ensure
	 * that in case the calling application crashes, memory dumps, pages
	 * out, or due to the CPU Jitter RNG lingering in memory for long
	 * time without being moved and an attacker cracks the application,
	 * all he reads in the entropy pool is a value that is NEVER EVER
	 * being used for anything. Thus, he does NOT see the previous value
	 * that was returned to the caller for cryptographic purposes.
	 */
	/*
	 * If we use secured memory, do not use that precaution as the secure
	 * memory protects the entropy pool. Moreover, note that using this
	 * call reduces the speed of the RNG by up to half
	 */
#ifndef CONFIG_CRYPTO_CPU_JITTERENTROPY_SECURE_MEMORY
	jent_random_data(ec);
#endif

err:
	jent_notime_unsettick(ec);
	return ret ? ret : (ssize_t)orig_len;
}

/***************************************************************************
 * Initialization logic
 ***************************************************************************/

JENT_PRIVATE_STATIC
struct rand_data *jent_entropy_collector_alloc(unsigned int osr,
					       unsigned int flags)
{
	struct rand_data *entropy_collector;

	/*
	 * Requesting disabling and forcing of internal timer
	 * makes no sense.
	 */
	if ((flags & JENT_DISABLE_INTERNAL_TIMER) &&
	    (flags & JENT_FORCE_INTERNAL_TIMER))
		return NULL;

	/*
	 * If the initial test code concludes to force the internal timer
	 * and the user requests it not to be used, do not allocate
	 * the Jitter RNG instance.
	 */
	if (jent_force_internal_timer && (flags & JENT_DISABLE_INTERNAL_TIMER))
		return NULL;

	entropy_collector = jent_zalloc(sizeof(struct rand_data));
	if (NULL == entropy_collector)
		return NULL;

	if (!(flags & JENT_DISABLE_MEMORY_ACCESS)) {
		/* Allocate memory for adding variations based on memory
		 * access
		 */
		entropy_collector->mem = 
			(unsigned char *)jent_zalloc(JENT_MEMORY_SIZE);
		if (entropy_collector->mem == NULL)
			goto err;

		entropy_collector->memblocksize = JENT_MEMORY_BLOCKSIZE;
		entropy_collector->memblocks = JENT_MEMORY_BLOCKS;
		entropy_collector->memaccessloops = JENT_MEMORY_ACCESSLOOPS;
	}

	/* verify and set the oversampling rate */
	if (osr < JENT_MIN_OSR)
		osr = JENT_MIN_OSR;
	entropy_collector->osr = osr;

	if (jent_fips_enabled() || (flags & JENT_FORCE_FIPS))
		entropy_collector->fips_enabled = 1;

	/* Use timer-less noise source */
	if (!(flags & JENT_DISABLE_INTERNAL_TIMER)) {
		if (jent_notime_enable(entropy_collector, flags))
			goto err;
	}

	/* fill the data pad with non-zero values */
	if (jent_notime_settick(entropy_collector))
		goto err;
	jent_random_data(entropy_collector);
	jent_notime_unsettick(entropy_collector);

	return entropy_collector;

err:
	if (entropy_collector->mem != NULL)
		jent_zfree(entropy_collector->mem, JENT_MEMORY_SIZE);
	jent_zfree(entropy_collector, sizeof(struct rand_data));
	return NULL;
}

JENT_PRIVATE_STATIC
void jent_entropy_collector_free(struct rand_data *entropy_collector)
{
	if (entropy_collector != NULL) {
		if (entropy_collector->mem != NULL) {
			jent_zfree(entropy_collector->mem, JENT_MEMORY_SIZE);
			entropy_collector->mem = NULL;
		}
		jent_zfree(entropy_collector, sizeof(struct rand_data));
	}
}

static int jent_time_entropy_init(unsigned int enable_notime)
{
	int i;
	uint64_t delta_sum = 0;
	uint64_t old_delta = 0;
	unsigned int nonstuck = 0;
	int time_backwards = 0;
	int count_mod = 0;
	int count_stuck = 0;
	int ret = 0;
	struct rand_data ec;

	memset(&ec, 0, sizeof(ec));

	if (enable_notime) {
		ec.enable_notime = 1;
		jent_notime_settick(&ec);
	}

	/* Required for RCT */
	ec.osr = 1;
	if (jent_fips_enabled())
		ec.fips_enabled = 1;

	/* We could perform statistical tests here, but the problem is
	 * that we only have a few loop counts to do testing. These
	 * loop counts may show some slight skew and we produce
	 * false positives.
	 *
	 * Moreover, only old systems show potentially problematic
	 * jitter entropy that could potentially be caught here. But
	 * the RNG is intended for hardware that is available or widely
	 * used, but not old systems that are long out of favor. Thus,
	 * no statistical tests.
	 */

	/*
	 * We could add a check for system capabilities such as clock_getres or
	 * check for CONFIG_X86_TSC, but it does not make much sense as the
	 * following sanity checks verify that we have a high-resolution
	 * timer.
	 */

#define CLEARCACHE 100
	for (i = 0; (JENT_POWERUP_TESTLOOPCOUNT + CLEARCACHE) > i; i++) {
		uint64_t time = 0;
		uint64_t time2 = 0;
		uint64_t delta = 0;
		unsigned int lowdelta = 0;
		unsigned int stuck;

		/* Invoke core entropy collection logic */
		jent_get_nstime_internal(&ec, &time);
		ec.prev_time = time;
		jent_memaccess(&ec, 0);
		jent_hash_time(&ec, time, 0, 0);
		jent_get_nstime_internal(&ec, &time2);

		/* test whether timer works */
		if (!time || !time2) {
			ret = ENOTIME;
			goto out;
		}
		delta = jent_delta(time, time2);
		/*
		 * test whether timer is fine grained enough to provide
		 * delta even when called shortly after each other -- this
		 * implies that we also have a high resolution timer
		 */
		if (!delta) {
			ret = ECOARSETIME;
			goto out;
		}

		stuck = jent_stuck(&ec, delta);

		/*
		 * up to here we did not modify any variable that will be
		 * evaluated later, but we already performed some work. Thus we
		 * already have had an impact on the caches, branch prediction,
		 * etc. with the goal to clear it to get the worst case
		 * measurements.
		 */
		if (CLEARCACHE > i)
			continue;

		if (stuck)
			count_stuck++;
		else {
			nonstuck++;

			/*
			 * Ensure that the APT succeeded.
			 *
			 * With the check below that count_stuck must be less
			 * than 10% of the overall generated raw entropy values
			 * it is guaranteed that the APT is invoked at
			 * floor((JENT_POWERUP_TESTLOOPCOUNT * 0.9) / 64) == 14
			 * times.
			 */
			if ((nonstuck % JENT_APT_WINDOW_SIZE) == 0) {
				jent_apt_reset(&ec,
					       delta & JENT_APT_WORD_MASK);
				if (jent_health_failure(&ec)) {
					ret = EHEALTH;
					goto out;
				}
			}
		}

		/* Validate RCT */
		if (jent_rct_failure(&ec)) {
			ret = ERCT;
			goto out;
		}

		/* test whether we have an increasing timer */
		if (!(time2 > time))
			time_backwards++;

		/* use 32 bit value to ensure compilation on 32 bit arches */
		lowdelta = (unsigned int)(time2 - time);
		if (!(lowdelta % 100))
			count_mod++;

		/*
		 * ensure that we have a varying delta timer which is necessary
		 * for the calculation of entropy -- perform this check
		 * only after the first loop is executed as we need to prime
		 * the old_data value
		 */
		if (delta > old_delta)
			delta_sum += (delta - old_delta);
		else
			delta_sum += (old_delta - delta);
		old_delta = delta;
	}

	/*
	 * we allow up to three times the time running backwards.
	 * CLOCK_REALTIME is affected by adjtime and NTP operations. Thus,
	 * if such an operation just happens to interfere with our test, it
	 * should not fail. The value of 3 should cover the NTP case being
	 * performed during our test run.
	 */
	if (time_backwards > 3) {
		ret = ENOMONOTONIC;
		goto out;
	}

	/*
	 * Variations of deltas of time must on average be larger
	 * than 1 to ensure the entropy estimation
	 * implied with 1 is preserved
	 */
	if ((delta_sum) <= JENT_POWERUP_TESTLOOPCOUNT) {
		ret = EMINVARVAR;
		goto out;
	}

	/*
	 * Ensure that we have variations in the time stamp below 10 for at
	 * least 10% of all checks -- on some platforms, the counter increments
	 * in multiples of 100, but not always
	 */
	if (JENT_STUCK_INIT_THRES(JENT_POWERUP_TESTLOOPCOUNT) < count_mod) {
		ret = ECOARSETIME;
		goto out;
	}

	/*
	 * If we have more than 90% stuck results, then this Jitter RNG is
	 * likely to not work well.
	 */
	if (JENT_STUCK_INIT_THRES(JENT_POWERUP_TESTLOOPCOUNT) < count_stuck)
		ret = ESTUCK;

out:
	if (enable_notime)
		jent_notime_unsettick(&ec);

	return ret;
}

JENT_PRIVATE_STATIC
int jent_entropy_init(void)
{
	int ret;

	if (sha3_tester())
		return EHASH;

	ret = jent_time_entropy_init(0);

#ifdef JENT_CONF_ENABLE_INTERNAL_TIMER
	jent_force_internal_timer = 0;
	if (ret) {
		ret = jent_time_entropy_init(1);
		if (!ret)
			jent_force_internal_timer = 1;
	}
#endif /* JENT_CONF_ENABLE_INTERNAL_TIMER */

	return ret;
}
