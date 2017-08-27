/*
 *  md4c.c	MD4 message-digest algorithm
 *
 *  Version:	$Id$
 *
 *  This file is licensed under the LGPL, but is largely derived
 *  from public domain source code.
 */

RCSID("$Id$")

/*
 *  FORCE MD4 TO USE OUR MD4 HEADER FILE!
 *  If we don't do this, it might pick up the systems broken MD4.
 */
#include "../include/md4.h"

void fr_md4_calc(output, input, inlen)
unsigned char *output;
const unsigned char *input;		       /* input block */
unsigned int inlen;		     /* length of input block */
{
	FR_MD4_CTX	context;

	fr_MD4Init(&context);
	fr_MD4Update(&context, input, inlen);
	fr_MD4Final(output, &context);
}

#ifndef WITH_OPENSSL_MD4
/*	The below was retrieved from
 *	http://www.openbsd.org/cgi-bin/cvsweb/~checkout~/src/lib/libc/hash/md4.c?rev=1.2
 *	with the following changes:
 *	CVS-$OpenBSD stuff deleted
 *	#includes commented out.
 *	Support context->count as uint32_t[2] instead of uint64_t
 *	Add htole32 define from http://www.squid-cache.org/mail-archive/squid-dev/200307/0130.html
 *		(The bswap32 definition in the patch.)
 *		This is only used on BIG_ENDIAN systems, so we can always swap the bits.
 *	change BYTE_ORDER == LITTLE_ENDIAN (OpenBSD-defined) to WORDS_BIGENDIAN (autoconf-defined)
 */

/*
 * This code implements the MD4 message-digest algorithm.
 * The algorithm is due to Ron Rivest.	This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 * Todd C. Miller modified the MD5 code to do MD4 based on RFC 1186.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD4Context structure, pass it to fr_MD4Init, call fr_MD4Update as
 * needed on buffers full of bytes, and then call fr_MD4Final, which
 * will fill a supplied 16-byte array with the digest.
 */

/*#include <sys/types.h>*/
/*#include <string.h>*/
/*#include <md4.h>*/

/*#if BYTE_ORDER == LITTLE_ENDIAN*/
#ifndef WORDS_BIGENDIAN

#define htole32_4(buf)		/* Nothing */
#define htole32_14(buf)		/* Nothing */
#define htole32_16(buf)		/* Nothing */

#else

#define htole32(x) \
 (((((uint32_t)x) & 0xff000000) >> 24) | \
 ((((uint32_t)x) & 0x00ff0000) >> 8) | \
 ((((uint32_t)x) & 0x0000ff00) << 8) | \
 ((((uint32_t)x) & 0x000000ff) << 24))

#define htole32_4(buf) do {						\
	(buf)[ 0] = htole32((buf)[ 0]);					\
	(buf)[ 1] = htole32((buf)[ 1]);					\
	(buf)[ 2] = htole32((buf)[ 2]);					\
	(buf)[ 3] = htole32((buf)[ 3]);					\
} while (0)

#define htole32_14(buf) do {						\
	(buf)[ 0] = htole32((buf)[ 0]);					\
	(buf)[ 1] = htole32((buf)[ 1]);					\
	(buf)[ 2] = htole32((buf)[ 2]);					\
	(buf)[ 3] = htole32((buf)[ 3]);					\
	(buf)[ 4] = htole32((buf)[ 4]);					\
	(buf)[ 5] = htole32((buf)[ 5]);					\
	(buf)[ 6] = htole32((buf)[ 6]);					\
	(buf)[ 7] = htole32((buf)[ 7]);					\
	(buf)[ 8] = htole32((buf)[ 8]);					\
	(buf)[ 9] = htole32((buf)[ 9]);					\
	(buf)[10] = htole32((buf)[10]);					\
	(buf)[11] = htole32((buf)[11]);					\
	(buf)[12] = htole32((buf)[12]);					\
	(buf)[13] = htole32((buf)[13]);					\
} while (0)

#define htole32_16(buf) do {						\
	(buf)[ 0] = htole32((buf)[ 0]);					\
	(buf)[ 1] = htole32((buf)[ 1]);					\
	(buf)[ 2] = htole32((buf)[ 2]);					\
	(buf)[ 3] = htole32((buf)[ 3]);					\
	(buf)[ 4] = htole32((buf)[ 4]);					\
	(buf)[ 5] = htole32((buf)[ 5]);					\
	(buf)[ 6] = htole32((buf)[ 6]);					\
	(buf)[ 7] = htole32((buf)[ 7]);					\
	(buf)[ 8] = htole32((buf)[ 8]);					\
	(buf)[ 9] = htole32((buf)[ 9]);					\
	(buf)[10] = htole32((buf)[10]);					\
	(buf)[11] = htole32((buf)[11]);					\
	(buf)[12] = htole32((buf)[12]);					\
	(buf)[13] = htole32((buf)[13]);					\
	(buf)[14] = htole32((buf)[14]);					\
	(buf)[15] = htole32((buf)[15]);					\
} while (0)

#endif

/*
 * Start MD4 accumulation.
 * Set bit count to 0 and buffer to mysterious initialization constants.
 */
void
fr_MD4Init(FR_MD4_CTX *ctx)
{
	ctx->count[0] = 0;
	ctx->count[1] = 0;
	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xefcdab89;
	ctx->state[2] = 0x98badcfe;
	ctx->state[3] = 0x10325476;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void
fr_MD4Update(FR_MD4_CTX *ctx, unsigned char const *buf, size_t len)
{
	uint32_t count;

	/* Bytes already stored in ctx->buffer */
	count = (uint32_t)((ctx->count[0] >> 3) & 0x3f);

	/* Update bitcount */
/*	ctx->count += (uint64_t)len << 3;*/
	if ((ctx->count[0] += ((uint32_t)len << 3)) < (uint32_t)len) {
	/* Overflowed ctx->count[0] */
		ctx->count[1]++;
	}
	ctx->count[1] += ((uint32_t)len >> 29);

	/* Handle any leading odd-sized chunks */
	if (count) {
		unsigned char *p = (unsigned char *)ctx->buffer + count;

		count = MD4_BLOCK_LENGTH - count;
		if (len < count) {
			memcpy(p, buf, len);
			return;
		}
		memcpy(p, buf, count);
		htole32_16((uint32_t *)ctx->buffer);
		fr_MD4Transform(ctx->state, ctx->buffer);
		buf += count;
		len -= count;
	}

	/* Process data in MD4_BLOCK_LENGTH-byte chunks */
	while (len >= MD4_BLOCK_LENGTH) {
		memcpy(ctx->buffer, buf, MD4_BLOCK_LENGTH);
		htole32_16((uint32_t *)ctx->buffer);
		fr_MD4Transform(ctx->state, ctx->buffer);
		buf += MD4_BLOCK_LENGTH;
		len -= MD4_BLOCK_LENGTH;
	}

	/* Handle any remaining bytes of data. */
	memcpy(ctx->buffer, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void
fr_MD4Final(unsigned char digest[MD4_DIGEST_LENGTH], FR_MD4_CTX *ctx)
{
	uint32_t count;
	unsigned char *p;

	/* number of bytes mod 64 */
	count = (uint32_t)(ctx->count[0] >> 3) & 0x3f;

	/*
	 * Set the first char of padding to 0x80.
	 * This is safe since there is always at least one byte free.
	 */
	p = ctx->buffer + count;
	*p++ = 0x80;

	/* Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;

	/* Pad out to 56 mod 64 */
	if (count < 8) {
		/* Two lots of padding:  Pad the first block to 64 bytes */
		memset(p, 0, count);
		htole32_16((uint32_t *)ctx->buffer);
		fr_MD4Transform(ctx->state, ctx->buffer);

		/* Now fill the next block with 56 bytes */
		memset(ctx->buffer, 0, 56);
	} else {
		/* Pad block to 56 bytes */
		memset(p, 0, count - 8);
	}
	htole32_14((uint32_t *)ctx->buffer);

	/* Append bit count and transform */
	((uint32_t *)ctx->buffer)[14] = ctx->count[0];
	((uint32_t *)ctx->buffer)[15] = ctx->count[1];

	fr_MD4Transform(ctx->state, ctx->buffer);
	htole32_4(ctx->state);
	memcpy(digest, ctx->state, MD4_DIGEST_LENGTH);
	memset(ctx, 0, sizeof(*ctx));	/* in case it's sensitive */
}


/* The three core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) ((x & y) | (x & z) | (y & z))
#define F3(x, y, z) (x ^ y ^ z)

/* This is the central step in the MD4 algorithm. */
#define MD4STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s) )

/*
 * The core of the MD4 algorithm, this alters an existing MD4 hash to
 * reflect the addition of 16 longwords of new data.  fr_MD4Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void
fr_MD4Transform(uint32_t buf[4], unsigned char const inc[MD4_BLOCK_LENGTH])
{
	uint32_t a, b, c, d;
	uint32_t const *in = (uint32_t const *)inc;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	MD4STEP(F1, a, b, c, d, in[ 0],  3);
	MD4STEP(F1, d, a, b, c, in[ 1],  7);
	MD4STEP(F1, c, d, a, b, in[ 2], 11);
	MD4STEP(F1, b, c, d, a, in[ 3], 19);
	MD4STEP(F1, a, b, c, d, in[ 4],  3);
	MD4STEP(F1, d, a, b, c, in[ 5],  7);
	MD4STEP(F1, c, d, a, b, in[ 6], 11);
	MD4STEP(F1, b, c, d, a, in[ 7], 19);
	MD4STEP(F1, a, b, c, d, in[ 8],  3);
	MD4STEP(F1, d, a, b, c, in[ 9],  7);
	MD4STEP(F1, c, d, a, b, in[10], 11);
	MD4STEP(F1, b, c, d, a, in[11], 19);
	MD4STEP(F1, a, b, c, d, in[12],  3);
	MD4STEP(F1, d, a, b, c, in[13],  7);
	MD4STEP(F1, c, d, a, b, in[14], 11);
	MD4STEP(F1, b, c, d, a, in[15], 19);

	MD4STEP(F2, a, b, c, d, in[ 0] + 0x5a827999,  3);
	MD4STEP(F2, d, a, b, c, in[ 4] + 0x5a827999,  5);
	MD4STEP(F2, c, d, a, b, in[ 8] + 0x5a827999,  9);
	MD4STEP(F2, b, c, d, a, in[12] + 0x5a827999, 13);
	MD4STEP(F2, a, b, c, d, in[ 1] + 0x5a827999,  3);
	MD4STEP(F2, d, a, b, c, in[ 5] + 0x5a827999,  5);
	MD4STEP(F2, c, d, a, b, in[ 9] + 0x5a827999,  9);
	MD4STEP(F2, b, c, d, a, in[13] + 0x5a827999, 13);
	MD4STEP(F2, a, b, c, d, in[ 2] + 0x5a827999,  3);
	MD4STEP(F2, d, a, b, c, in[ 6] + 0x5a827999,  5);
	MD4STEP(F2, c, d, a, b, in[10] + 0x5a827999,  9);
	MD4STEP(F2, b, c, d, a, in[14] + 0x5a827999, 13);
	MD4STEP(F2, a, b, c, d, in[ 3] + 0x5a827999,  3);
	MD4STEP(F2, d, a, b, c, in[ 7] + 0x5a827999,  5);
	MD4STEP(F2, c, d, a, b, in[11] + 0x5a827999,  9);
	MD4STEP(F2, b, c, d, a, in[15] + 0x5a827999, 13);

	MD4STEP(F3, a, b, c, d, in[ 0] + 0x6ed9eba1,  3);
	MD4STEP(F3, d, a, b, c, in[ 8] + 0x6ed9eba1,  9);
	MD4STEP(F3, c, d, a, b, in[ 4] + 0x6ed9eba1, 11);
	MD4STEP(F3, b, c, d, a, in[12] + 0x6ed9eba1, 15);
	MD4STEP(F3, a, b, c, d, in[ 2] + 0x6ed9eba1,  3);
	MD4STEP(F3, d, a, b, c, in[10] + 0x6ed9eba1,  9);
	MD4STEP(F3, c, d, a, b, in[ 6] + 0x6ed9eba1, 11);
	MD4STEP(F3, b, c, d, a, in[14] + 0x6ed9eba1, 15);
	MD4STEP(F3, a, b, c, d, in[ 1] + 0x6ed9eba1,  3);
	MD4STEP(F3, d, a, b, c, in[ 9] + 0x6ed9eba1,  9);
	MD4STEP(F3, c, d, a, b, in[ 5] + 0x6ed9eba1, 11);
	MD4STEP(F3, b, c, d, a, in[13] + 0x6ed9eba1, 15);
	MD4STEP(F3, a, b, c, d, in[ 3] + 0x6ed9eba1,  3);
	MD4STEP(F3, d, a, b, c, in[11] + 0x6ed9eba1,  9);
	MD4STEP(F3, c, d, a, b, in[ 7] + 0x6ed9eba1, 11);
	MD4STEP(F3, b, c, d, a, in[15] + 0x6ed9eba1, 15);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}
#endif
