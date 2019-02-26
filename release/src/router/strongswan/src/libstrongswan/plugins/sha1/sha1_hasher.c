/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Ported from Steve Reid's <steve@edmweb.com> implementation
 * "SHA1 in C" found in strongSwan.
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

#include <library.h>

#include "sha1_hasher.h"

/*
 * ugly macro stuff
 */
#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#if BYTE_ORDER == LITTLE_ENDIAN
 #define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) |(rol(block->l[i],8)&0x00FF00FF))
#elif BYTE_ORDER == BIG_ENDIAN
 #define blk0(i) block->l[i]
#else
 #error "Endianness not defined!"
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


typedef struct private_sha1_hasher_t private_sha1_hasher_t;

/**
 * Private data structure with hasing context.
 */
struct private_sha1_hasher_t {
	/**
	 * Public interface for this hasher.
	 */
	sha1_hasher_t public;

	/*
	 * State of the hasher. Shared with sha1_prf.c, do not change it!!!
	 */
	uint32_t state[5];
	uint32_t count[2];
	uint8_t buffer[64];
};

/*
 * Hash a single 512-bit block. This is the core of the algorithm. *
 */
static void SHA1Transform(uint32_t state[5], const unsigned char buffer[64])
{
	uint32_t a, b, c, d, e;
	typedef union {
		uint8_t c[64];
		uint32_t l[16];
	} CHAR64LONG16;
	CHAR64LONG16 block[1];  /* use array to appear as a pointer */
	memcpy(block, buffer, 64);

	/* Copy context->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	/* Wipe variables */
	a = b = c = d = e = 0;
	memset(block, '\0', sizeof(block));
}

/**
 * Run your data through this. Also used in sha1_prf.
 */
void SHA1Update(private_sha1_hasher_t* this, uint8_t *data, uint32_t len)
{
	uint32_t i;
	uint32_t j;

	j = this->count[0];
	if ((this->count[0] += len << 3) < j)
	{
		this->count[1]++;
	}
	this->count[1] += (len>>29);
	j = (j >> 3) & 63;
	if ((j + len) > 63)
	{
		memcpy(&this->buffer[j], data, (i = 64-j));
		SHA1Transform(this->state, this->buffer);
		for ( ; i + 63 < len; i += 64)
		{
		    SHA1Transform(this->state, &data[i]);
		}
		j = 0;
	}
	else
	{
		i = 0;
	}
	memcpy(&this->buffer[j], &data[i], len - i);
}


/*
 * Add padding and return the message digest.
 */
static void SHA1Final(private_sha1_hasher_t *this, uint8_t *digest)
{
	uint32_t i;
	uint8_t finalcount[8];
	uint8_t c;

	for (i = 0; i < 8; i++)
	{
		finalcount[i] = (uint8_t)((this->count[(i >= 4 ? 0 : 1)]
		 >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
	}
	c = 0200;
	SHA1Update(this, &c, 1);
	while ((this->count[0] & 504) != 448)
	{
		c = 0000;
		SHA1Update(this, &c, 1);
	}
	SHA1Update(this, finalcount, 8);  /* Should cause a SHA1Transform() */
	for (i = 0; i < 20; i++)
	{
		digest[i] = (uint8_t)((this->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
	}
}

METHOD(hasher_t, reset, bool,
	private_sha1_hasher_t *this)
{
	this->state[0] = 0x67452301;
	this->state[1] = 0xEFCDAB89;
	this->state[2] = 0x98BADCFE;
	this->state[3] = 0x10325476;
	this->state[4] = 0xC3D2E1F0;
	this->count[0] = 0;
	this->count[1] = 0;

	return TRUE;
}

METHOD(hasher_t, get_hash, bool,
	private_sha1_hasher_t *this, chunk_t chunk, uint8_t *buffer)
{
	SHA1Update(this, chunk.ptr, chunk.len);
	if (buffer != NULL)
	{
		SHA1Final(this, buffer);
		reset(this);
	}
	return TRUE;
}

METHOD(hasher_t, allocate_hash, bool,
	private_sha1_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	SHA1Update(this, chunk.ptr, chunk.len);
	if (hash != NULL)
	{
		hash->ptr = malloc(HASH_SIZE_SHA1);
		hash->len = HASH_SIZE_SHA1;

		SHA1Final(this, hash->ptr);
		reset(this);
	}
	return TRUE;
}

METHOD(hasher_t, get_hash_size, size_t,
	private_sha1_hasher_t *this)
{
	return HASH_SIZE_SHA1;
}

METHOD(hasher_t, destroy, void,
	private_sha1_hasher_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
sha1_hasher_t *sha1_hasher_create(hash_algorithm_t algo)
{
	private_sha1_hasher_t *this;

	if (algo != HASH_SHA1)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.hasher_interface = {
				.get_hash = _get_hash,
				.allocate_hash = _allocate_hash,
				.get_hash_size = _get_hash_size,
				.reset = _reset,
				.destroy = _destroy,
			},
		},
	);

	/* initialize */
	reset(this);

	return &(this->public);
}
