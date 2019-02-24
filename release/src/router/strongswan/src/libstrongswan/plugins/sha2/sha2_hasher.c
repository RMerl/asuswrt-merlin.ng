/*
 * Copyright (C) 2006 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 * Copyright (C) 2001 Jari Ruusu.
 *
 * Ported from strongSwans implementation written by Jari Ruusu.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <string.h>

#include "sha2_hasher.h"


typedef struct private_sha512_hasher_t private_sha512_hasher_t;

/**
 * Private data structure with hasing context for SHA384 and SHA512
 */
struct private_sha512_hasher_t {
	/**
	 * Public interface for this hasher.
	 */
	sha2_hasher_t public;

	unsigned char   sha_out[128];   /* results are here, bytes 0..47/0..63 */
	uint64_t       sha_H[8];
	uint64_t       sha_blocks;
	uint64_t       sha_blocksMSB;
	int             sha_bufCnt;
};


typedef struct private_sha256_hasher_t private_sha256_hasher_t;

/**
 * Private data structure with hasing context for SHA256
 */
struct private_sha256_hasher_t {
	/**
	 * Public interface for this hasher.
	 */
	sha2_hasher_t public;

	unsigned char   sha_out[64];    /* results are here, bytes 0...31 */
	uint32_t       sha_H[8];
	uint64_t       sha_blocks;
	int             sha_bufCnt;
};


static const uint32_t sha224_hashInit[8] = {
	0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939, 0xffc00b31, 0x68581511,
	0x64f98fa7, 0xbefa4fa4
};

static const uint32_t sha256_hashInit[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c,
	0x1f83d9ab, 0x5be0cd19
};

static const uint32_t sha256_K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
	0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
	0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
	0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
	0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static const uint64_t sha512_hashInit[8] = {
	0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL,
	0xa54ff53a5f1d36f1ULL, 0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
	0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

static const uint64_t sha384_hashInit[8] = {
	0xcbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL, 0x9159015a3070dd17ULL,
	0x152fecd8f70e5939ULL, 0x67332667ffc00b31ULL, 0x8eb44a8768581511ULL,
	0xdb0c2e0d64f98fa7ULL, 0x47b5481dbefa4fa4ULL
};

static const uint64_t sha512_K[80] = {
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL,
	0xe9b5dba58189dbbcULL, 0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
	0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL, 0xd807aa98a3030242ULL,
	0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL,
	0xc19bf174cf692694ULL, 0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
	0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL, 0x2de92c6f592b0275ULL,
	0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL,
	0xbf597fc7beef0ee4ULL, 0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
	0x06ca6351e003826fULL, 0x142929670a0e6e70ULL, 0x27b70a8546d22ffcULL,
	0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
	0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL,
	0x92722c851482353bULL, 0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
	0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 0xd192e819d6ef5218ULL,
	0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL,
	0x34b0bcb5e19b48a8ULL, 0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
	0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL, 0x748f82ee5defb2fcULL,
	0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL,
	0xc67178f2e372532bULL, 0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
	0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 0x06f067aa72176fbaULL,
	0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
	0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL,
	0x431d67c49c100d4cULL, 0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
	0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};


/* set macros for SHA256 */
#define Ch(x,y,z)   (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define R(x,y)      ((y) >> (x))

#define S(x,y)      (((y) >> (x)) | ((y) << (32 - (x))))
#define uSig0(x)    ((S(2,(x))) ^ (S(13,(x))) ^ (S(22,(x))))
#define uSig1(x)    ((S(6,(x))) ^ (S(11,(x))) ^ (S(25,(x))))
#define lSig0(x)    ((S(7,(x))) ^ (S(18,(x))) ^ (R(3,(x))))
#define lSig1(x)    ((S(17,(x))) ^ (S(19,(x))) ^ (R(10,(x))))

/**
 * Single block SHA256 transformation
 */
static void sha256_transform(private_sha256_hasher_t *ctx,
							 const unsigned char *datap)
{
	register int    j;
	uint32_t       a, b, c, d, e, f, g, h;
	uint32_t       T1, T2, W[64], Wm2, Wm15;

	/* read the data, big endian byte order */
	j = 0;
	do {
		W[j] = (((uint32_t)(datap[0]))<<24) | (((uint32_t)(datap[1]))<<16) |
				(((uint32_t)(datap[2]))<<8 ) | ((uint32_t)(datap[3]));
		datap += 4;
	} while(++j < 16);

	/* initialize variables a...h */
	a = ctx->sha_H[0];
	b = ctx->sha_H[1];
	c = ctx->sha_H[2];
	d = ctx->sha_H[3];
	e = ctx->sha_H[4];
	f = ctx->sha_H[5];
	g = ctx->sha_H[6];
	h = ctx->sha_H[7];

	/* apply compression function */
	j = 0;
	do
	{
		if(j >= 16)
		{
			Wm2 = W[j - 2];
			Wm15 = W[j - 15];
			W[j] = lSig1(Wm2) + W[j - 7] + lSig0(Wm15) + W[j - 16];
		}
		T1 = h + uSig1(e) + Ch(e,f,g) + sha256_K[j] + W[j];
		T2 = uSig0(a) + Maj(a,b,c);
		h = g; g = f; f = e;
		e = d + T1;
		d = c; c = b; b = a;
		a = T1 + T2;
	} while(++j < 64);

	/* compute intermediate hash value */
	ctx->sha_H[0] += a;
	ctx->sha_H[1] += b;
	ctx->sha_H[2] += c;
	ctx->sha_H[3] += d;
	ctx->sha_H[4] += e;
	ctx->sha_H[5] += f;
	ctx->sha_H[6] += g;
	ctx->sha_H[7] += h;

	ctx->sha_blocks++;
}

/**
 * Update SHA256 hash
 */
static void sha256_write(private_sha256_hasher_t *ctx,
						 const unsigned char *datap, int length)
{
	while(length > 0)
	{
		if(!ctx->sha_bufCnt)
		{
			while(length >= sizeof(ctx->sha_out))
			{
				sha256_transform(ctx, datap);
				datap += sizeof(ctx->sha_out);
				length -= sizeof(ctx->sha_out);
			}
			if(!length) return;
		}
		ctx->sha_out[ctx->sha_bufCnt] = *datap++;
		length--;
		if(++ctx->sha_bufCnt == sizeof(ctx->sha_out))
		{
			sha256_transform(ctx, &ctx->sha_out[0]);
			ctx->sha_bufCnt = 0;
		}
	}
}

/**
 * finalize SHA256 hash
 */
static void sha256_final(private_sha256_hasher_t *ctx, u_char *buf, size_t len)
{
	register int    j;
	uint64_t       bitLength;
	uint32_t       i;
	unsigned char   padByte, *datap;

	bitLength = (ctx->sha_blocks << 9) | (ctx->sha_bufCnt << 3);
	padByte = 0x80;
	sha256_write(ctx, &padByte, 1);

	/* pad extra space with zeroes */
	padByte = 0;
	while(ctx->sha_bufCnt != 56)
	{
		sha256_write(ctx, &padByte, 1);
	}

	/* write bit length, big endian byte order */
	ctx->sha_out[56] = bitLength >> 56;
	ctx->sha_out[57] = bitLength >> 48;
	ctx->sha_out[58] = bitLength >> 40;
	ctx->sha_out[59] = bitLength >> 32;
	ctx->sha_out[60] = bitLength >> 24;
	ctx->sha_out[61] = bitLength >> 16;
	ctx->sha_out[62] = bitLength >> 8;
	ctx->sha_out[63] = bitLength;
	sha256_transform(ctx, &ctx->sha_out[0]);

	datap = buf;
	j = 0;
	do {
		i = ctx->sha_H[j];
		datap[0] = i >> 24;
		datap[1] = i >> 16;
		datap[2] = i >> 8;
		datap[3] = i;
		datap += 4;
	} while(++j < len / 4);
}

/* update macros for SHA512 */
#undef S
#undef uSig0
#undef uSig1
#undef lSig0
#undef lSig1
#define S(x,y)      (((y) >> (x)) | ((y) << (64 - (x))))
#define uSig0(x)    ((S(28,(x))) ^ (S(34,(x))) ^ (S(39,(x))))
#define uSig1(x)    ((S(14,(x))) ^ (S(18,(x))) ^ (S(41,(x))))
#define lSig0(x)    ((S(1,(x))) ^ (S(8,(x))) ^ (R(7,(x))))
#define lSig1(x)    ((S(19,(x))) ^ (S(61,(x))) ^ (R(6,(x))))

/**
 * Single block SHA384/SHA512 transformation
 */
static void sha512_transform(private_sha512_hasher_t *ctx,
							 const unsigned char *datap)
{
	register int    j;
	uint64_t       a, b, c, d, e, f, g, h;
	uint64_t       T1, T2, W[80], Wm2, Wm15;

	/* read the data, big endian byte order */
	j = 0;
	do {
		W[j] = (((uint64_t)(datap[0]))<<56) | (((uint64_t)(datap[1]))<<48) |
				(((uint64_t)(datap[2]))<<40) | (((uint64_t)(datap[3]))<<32) |
				(((uint64_t)(datap[4]))<<24) | (((uint64_t)(datap[5]))<<16) |
				(((uint64_t)(datap[6]))<<8 ) | ((uint64_t)(datap[7]));
		datap += 8;
	} while(++j < 16);

	/* initialize variables a...h */
	a = ctx->sha_H[0];
	b = ctx->sha_H[1];
	c = ctx->sha_H[2];
	d = ctx->sha_H[3];
	e = ctx->sha_H[4];
	f = ctx->sha_H[5];
	g = ctx->sha_H[6];
	h = ctx->sha_H[7];

	/* apply compression function */
	j = 0;
	do {
		if(j >= 16) {
			Wm2 = W[j - 2];
			Wm15 = W[j - 15];
			W[j] = lSig1(Wm2) + W[j - 7] + lSig0(Wm15) + W[j - 16];
		}
		T1 = h + uSig1(e) + Ch(e,f,g) + sha512_K[j] + W[j];
		T2 = uSig0(a) + Maj(a,b,c);
		h = g; g = f; f = e;
		e = d + T1;
		d = c; c = b; b = a;
		a = T1 + T2;
	} while(++j < 80);

	/* compute intermediate hash value */
	ctx->sha_H[0] += a;
	ctx->sha_H[1] += b;
	ctx->sha_H[2] += c;
	ctx->sha_H[3] += d;
	ctx->sha_H[4] += e;
	ctx->sha_H[5] += f;
	ctx->sha_H[6] += g;
	ctx->sha_H[7] += h;

	ctx->sha_blocks++;
	if(!ctx->sha_blocks) ctx->sha_blocksMSB++;
}

/**
 * Update a SHA384/SHA512 hash
 */
static void sha512_write(private_sha512_hasher_t *ctx,
						 const unsigned char *datap, int length)
{
	while(length > 0)
	{
		if(!ctx->sha_bufCnt)
		{
			while(length >= sizeof(ctx->sha_out))
			{
				sha512_transform(ctx, datap);
				datap += sizeof(ctx->sha_out);
				length -= sizeof(ctx->sha_out);
			}
			if(!length) return;
		}
		ctx->sha_out[ctx->sha_bufCnt] = *datap++;
		length--;
		if(++ctx->sha_bufCnt == sizeof(ctx->sha_out))
		{
			sha512_transform(ctx, &ctx->sha_out[0]);
			ctx->sha_bufCnt = 0;
		}
	}
}

/**
 * Finalize a SHA384/SHA512 hash
 */
static void sha512_final(private_sha512_hasher_t *ctx, u_char *buf, size_t len)
{
	register int    j;
	uint64_t       bitLength, bitLengthMSB;
	uint64_t       i;
	unsigned char   padByte, *datap;

	bitLength = (ctx->sha_blocks << 10) | (ctx->sha_bufCnt << 3);
	bitLengthMSB = (ctx->sha_blocksMSB << 10) | (ctx->sha_blocks >> 54);
	padByte = 0x80;
	sha512_write(ctx, &padByte, 1);

	/* pad extra space with zeroes */
	padByte = 0;
	while(ctx->sha_bufCnt != 112)
	{
		sha512_write(ctx, &padByte, 1);
	}

	/* write bit length, big endian byte order */
	ctx->sha_out[112] = bitLengthMSB >> 56;
	ctx->sha_out[113] = bitLengthMSB >> 48;
	ctx->sha_out[114] = bitLengthMSB >> 40;
	ctx->sha_out[115] = bitLengthMSB >> 32;
	ctx->sha_out[116] = bitLengthMSB >> 24;
	ctx->sha_out[117] = bitLengthMSB >> 16;
	ctx->sha_out[118] = bitLengthMSB >> 8;
	ctx->sha_out[119] = bitLengthMSB;
	ctx->sha_out[120] = bitLength >> 56;
	ctx->sha_out[121] = bitLength >> 48;
	ctx->sha_out[122] = bitLength >> 40;
	ctx->sha_out[123] = bitLength >> 32;
	ctx->sha_out[124] = bitLength >> 24;
	ctx->sha_out[125] = bitLength >> 16;
	ctx->sha_out[126] = bitLength >> 8;
	ctx->sha_out[127] = bitLength;
	sha512_transform(ctx, &ctx->sha_out[0]);

	datap = buf;
	j = 0;
	do {
		i = ctx->sha_H[j];
		datap[0] = i >> 56;
		datap[1] = i >> 48;
		datap[2] = i >> 40;
		datap[3] = i >> 32;
		datap[4] = i >> 24;
		datap[5] = i >> 16;
		datap[6] = i >> 8;
		datap[7] = i;
		datap += 8;
	} while(++j < len / 8);
}

METHOD(hasher_t, reset224, bool,
	private_sha256_hasher_t *this)
{
	memcpy(&this->sha_H[0], &sha224_hashInit[0], sizeof(this->sha_H));
	this->sha_blocks = 0;
	this->sha_bufCnt = 0;
	return TRUE;
}

METHOD(hasher_t, reset256, bool,
	private_sha256_hasher_t *this)
{
	memcpy(&this->sha_H[0], &sha256_hashInit[0], sizeof(this->sha_H));
	this->sha_blocks = 0;
	this->sha_bufCnt = 0;
	return TRUE;
}

METHOD(hasher_t, reset384, bool,
	private_sha512_hasher_t *this)
{
	memcpy(&this->sha_H[0], &sha384_hashInit[0], sizeof(this->sha_H));
	this->sha_blocks = 0;
	this->sha_blocksMSB = 0;
	this->sha_bufCnt = 0;
	return TRUE;
}

METHOD(hasher_t, reset512, bool,
	private_sha512_hasher_t *this)
{
	memcpy(&this->sha_H[0], &sha512_hashInit[0], sizeof(this->sha_H));
	this->sha_blocks = 0;
	this->sha_blocksMSB = 0;
	this->sha_bufCnt = 0;
	return TRUE;
}

METHOD(hasher_t, get_hash224, bool,
	private_sha256_hasher_t *this, chunk_t chunk, uint8_t *buffer)
{
	sha256_write(this, chunk.ptr, chunk.len);
	if (buffer != NULL)
	{
		sha256_final(this, buffer, HASH_SIZE_SHA224);
		reset224(this);
	}
	return TRUE;
}

METHOD(hasher_t, get_hash256, bool,
	private_sha256_hasher_t *this, chunk_t chunk, uint8_t *buffer)
{
	sha256_write(this, chunk.ptr, chunk.len);
	if (buffer != NULL)
	{
		sha256_final(this, buffer, HASH_SIZE_SHA256);
		reset256(this);
	}
	return TRUE;
}

METHOD(hasher_t, get_hash384, bool,
	private_sha512_hasher_t *this, chunk_t chunk, uint8_t *buffer)
{
	sha512_write(this, chunk.ptr, chunk.len);
	if (buffer != NULL)
	{
		sha512_final(this, buffer, HASH_SIZE_SHA384);
		reset384(this);
	}
	return TRUE;
}

METHOD(hasher_t, get_hash512, bool,
	private_sha512_hasher_t *this, chunk_t chunk, uint8_t *buffer)
{
	sha512_write(this, chunk.ptr, chunk.len);
	if (buffer != NULL)
	{
		sha512_final(this, buffer, HASH_SIZE_SHA512);
		reset512(this);
	}
	return TRUE;
}

METHOD(hasher_t, allocate_hash224, bool,
	private_sha256_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	chunk_t allocated_hash = chunk_empty;

	if (hash)
	{
		*hash = allocated_hash = chunk_alloc(HASH_SIZE_SHA224);
	}
	return get_hash224(this, chunk, allocated_hash.ptr);
}

METHOD(hasher_t, allocate_hash256, bool,
	private_sha256_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	chunk_t allocated_hash = chunk_empty;

	if (hash)
	{
		*hash = allocated_hash = chunk_alloc(HASH_SIZE_SHA256);
	}
	return get_hash256(this, chunk, allocated_hash.ptr);
}

METHOD(hasher_t, allocate_hash384, bool,
	private_sha512_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	chunk_t allocated_hash = chunk_empty;

	if (hash)
	{
		*hash = allocated_hash = chunk_alloc(HASH_SIZE_SHA384);
	}
	return get_hash384(this, chunk, allocated_hash.ptr);
}

METHOD(hasher_t, allocate_hash512, bool,
	private_sha512_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	chunk_t allocated_hash = chunk_empty;

	if (hash)
	{
		*hash = allocated_hash = chunk_alloc(HASH_SIZE_SHA512);
	}
	return get_hash512(this, chunk, allocated_hash.ptr);
}

METHOD(hasher_t, get_hash_size224, size_t,
	private_sha256_hasher_t *this)
{
	return HASH_SIZE_SHA224;
}

METHOD(hasher_t, get_hash_size256, size_t,
	private_sha256_hasher_t *this)
{
	return HASH_SIZE_SHA256;
}

METHOD(hasher_t, get_hash_size384, size_t,
	private_sha512_hasher_t *this)
{
	return HASH_SIZE_SHA384;
}

METHOD(hasher_t, get_hash_size512, size_t,
	private_sha512_hasher_t *this)
{
	return HASH_SIZE_SHA512;
}

METHOD(hasher_t, destroy, void,
	sha2_hasher_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
sha2_hasher_t *sha2_hasher_create(hash_algorithm_t algorithm)
{
	switch (algorithm)
	{
		case HASH_SHA224:
		{
			private_sha256_hasher_t *this;

			INIT(this,
				.public = {
					.hasher_interface = {
						.reset = _reset224,
						.get_hash_size = _get_hash_size224,
						.get_hash = _get_hash224,
						.allocate_hash = _allocate_hash224,
						.destroy = _destroy,
					},
				},
			);
			reset224(this);
			return &this->public;
		}
		case HASH_SHA256:
		{
			private_sha256_hasher_t *this;

			INIT(this,
				.public = {
					.hasher_interface = {
					.reset = _reset256,
					.get_hash_size = _get_hash_size256,
					.get_hash = _get_hash256,
					.allocate_hash = _allocate_hash256,
					.destroy = _destroy,
					},
				},
			);
			reset256(this);
			return &this->public;
		}
		case HASH_SHA384:
		{
			private_sha512_hasher_t *this;

			INIT(this,
				.public = {
					.hasher_interface = {
					.reset = _reset384,
					.get_hash_size = _get_hash_size384,
					.get_hash = _get_hash384,
					.allocate_hash = _allocate_hash384,
					.destroy = _destroy,
					},
				},
			);
			reset384(this);
			return &this->public;
		}
		case HASH_SHA512:
		{
			private_sha512_hasher_t *this;

			INIT(this,
				.public = {
					.hasher_interface = {
					.reset = _reset512,
					.get_hash_size = _get_hash_size512,
					.get_hash = _get_hash512,
					.allocate_hash = _allocate_hash512,
					.destroy = _destroy,
					},
				},
			);
			reset512(this);
			return &this->public;
		}
		default:
			return NULL;
	}
}
