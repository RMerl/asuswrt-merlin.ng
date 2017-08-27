/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "gcm_aead.h"

#include <limits.h>
#include <crypto/iv/iv_gen_seq.h>

#define BLOCK_SIZE 16
#define NONCE_SIZE 12
#define IV_SIZE 8
#define SALT_SIZE (NONCE_SIZE - IV_SIZE)

typedef struct private_gcm_aead_t private_gcm_aead_t;

/**
 * Private data of an gcm_aead_t object.
 */
struct private_gcm_aead_t {

	/**
	 * Public gcm_aead_t interface.
	 */
	gcm_aead_t public;

	/**
	 * Underlying CBC crypter.
	 */
	crypter_t *crypter;

	/**
	 * IV generator.
	 */
	iv_gen_t *iv_gen;

	/**
	 * Size of the integrity check value
	 */
	size_t icv_size;

	/**
	 * Salt value
	 */
	char salt[SALT_SIZE];

	/**
	 * GHASH subkey H
	 */
	char h[BLOCK_SIZE];
};

/**
 * Find a suiteable word size and network order conversion functions
 */
#if ULONG_MAX == 18446744073709551615UL && defined(htobe64)
#	define htobeword htobe64
#	define bewordtoh be64toh
#	define SHIFT_WORD_TYPE u_int64_t
#else
#	define htobeword htonl
#	define bewordtoh ntohl
#	define SHIFT_WORD_TYPE u_int32_t
#endif

/**
 * Bitshift a block right by one bit
 */
static void sr_block(char *block)
{
	int i;
	SHIFT_WORD_TYPE *word = (SHIFT_WORD_TYPE*)block;

	for (i = 0; i < BLOCK_SIZE / sizeof(*word); i++)
	{
		word[i] = bewordtoh(word[i]);
	}
	for (i = BLOCK_SIZE / sizeof(*word) - 1; i >= 0; i--)
	{
		word[i] >>= 1;
		if (i != 0)
		{
			word[i] |= word[i - 1] << (sizeof(*word) * 8 - 1);
		}
	}
	for (i = 0; i < BLOCK_SIZE / sizeof(*word); i++)
	{
		word[i] = htobeword(word[i]);
	}
}

/**
 * Naive implementation of block multiplication in GF128, no tables
 */
static void mult_block(char *x, char *y, char *res)
{
	char z[BLOCK_SIZE], v[BLOCK_SIZE], r;
	int bit, byte;

	r = 0xE1;
	memset(z, 0, BLOCK_SIZE);
	memcpy(v, y, BLOCK_SIZE);

	for (byte = 0; byte < BLOCK_SIZE; byte++)
	{
		for (bit = 7; bit >= 0; bit--)
		{
			if (x[byte] & (1 << bit))
			{
				memxor(z, v, BLOCK_SIZE);
			}
			if (v[BLOCK_SIZE - 1] & 0x01)
			{
				sr_block(v);
				v[0] ^= r;
			}
			else
			{
				sr_block(v);
			}
		}
	}
	memcpy(res, z, BLOCK_SIZE);
}

/**
 * GHASH function
 */
static void ghash(private_gcm_aead_t *this, chunk_t x, char *res)
{
	char y[BLOCK_SIZE];

	memset(y, 0, BLOCK_SIZE);

	while (x.len)
	{
		memxor(y, x.ptr, BLOCK_SIZE);
		mult_block(y, this->h, y);
		x = chunk_skip(x, BLOCK_SIZE);
	}
	memcpy(res, y, BLOCK_SIZE);
}

/**
 * GCTR function, en-/decrypts x inline
 */
static bool gctr(private_gcm_aead_t *this, char *icb, chunk_t x)
{
	char cb[BLOCK_SIZE], iv[BLOCK_SIZE], tmp[BLOCK_SIZE];

	memset(iv, 0, BLOCK_SIZE);
	memcpy(cb, icb, BLOCK_SIZE);

	while (x.len)
	{
		memcpy(tmp, cb, BLOCK_SIZE);
		if (!this->crypter->encrypt(this->crypter, chunk_from_thing(tmp),
									chunk_from_thing(iv), NULL))
		{
			return FALSE;
		}
		memxor(x.ptr, tmp, min(BLOCK_SIZE, x.len));
		chunk_increment(chunk_from_thing(cb));
		x = chunk_skip(x, BLOCK_SIZE);
	}
	return TRUE;
}

/**
 * Generate the block J0
 */
static void create_j(private_gcm_aead_t *this, char *iv, char *j)
{
	memcpy(j, this->salt, SALT_SIZE);
	memcpy(j + SALT_SIZE, iv, IV_SIZE);
	htoun32(j + SALT_SIZE + IV_SIZE, 1);
}

/**
 * Create GHASH subkey H
 */
static bool create_h(private_gcm_aead_t *this, char *h)
{
	char zero[BLOCK_SIZE];

	memset(zero, 0, BLOCK_SIZE);
	memset(h, 0, BLOCK_SIZE);

	return this->crypter->encrypt(this->crypter, chunk_create(h, BLOCK_SIZE),
								  chunk_from_thing(zero), NULL);
}

/**
 * Encrypt/decrypt
 */
static bool crypt(private_gcm_aead_t *this, char *j, chunk_t in, chunk_t out)
{
	char icb[BLOCK_SIZE];

	memcpy(icb, j, BLOCK_SIZE);
	chunk_increment(chunk_from_thing(icb));

	out.len = in.len;
	if (in.ptr != out.ptr)
	{
		memcpy(out.ptr, in.ptr, in.len);
	}
	return gctr(this, icb, out);
}

/**
 * Create ICV
 */
static bool create_icv(private_gcm_aead_t *this, chunk_t assoc, chunk_t crypt,
					   char *j, char *icv)
{
	size_t assoc_pad, crypt_pad;
	chunk_t chunk;
	char s[BLOCK_SIZE], *pos;

	assoc_pad = (BLOCK_SIZE - (assoc.len % BLOCK_SIZE)) % BLOCK_SIZE;
	crypt_pad = (BLOCK_SIZE - (crypt.len % BLOCK_SIZE)) % BLOCK_SIZE;

	/* concatenate data to a new chunk */
	chunk = chunk_alloc(assoc.len + assoc_pad +
						crypt.len + crypt_pad + BLOCK_SIZE);
	pos = chunk.ptr;
	/* add associated data */
	memcpy(pos, assoc.ptr, assoc.len);
	pos += assoc.len;
	memset(pos, 0, assoc_pad);
	pos += assoc_pad;
	/* add encrypted data */
	memcpy(pos, crypt.ptr, crypt.len);
	pos += crypt.len;
	memset(pos, 0, crypt_pad);
	pos += crypt_pad;
	/* write associated len */
	memset(pos, 0, 4);
	pos += 4;
	htoun32(pos, assoc.len * 8);
	pos += 4;
	/* write encrypted length */
	memset(pos, 0, 4);
	pos += 4;
	htoun32(pos, crypt.len * 8);
	pos += 4;

	ghash(this, chunk, s);
	free(chunk.ptr);
	if (!gctr(this, j, chunk_from_thing(s)))
	{
		return FALSE;
	}
	memcpy(icv, s, this->icv_size);
	return TRUE;
}

/**
 * Verify the ICV value
 */
static bool verify_icv(private_gcm_aead_t *this, chunk_t assoc, chunk_t crypt,
					   char *j, char *icv)
{
	char tmp[this->icv_size];

	return create_icv(this, assoc, crypt, j, tmp) &&
		   memeq(tmp, icv, this->icv_size);
}

METHOD(aead_t, encrypt, bool,
	private_gcm_aead_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
	chunk_t *encrypted)
{
	char j[BLOCK_SIZE];

	create_j(this, iv.ptr, j);

	if (encrypted)
	{
		*encrypted = chunk_alloc(plain.len + this->icv_size);
		return crypt(this, j, plain, *encrypted) &&
			   create_icv(this, assoc,
					chunk_create(encrypted->ptr, encrypted->len - this->icv_size),
					j, encrypted->ptr + encrypted->len - this->icv_size);
	}
	return crypt(this, j, plain, plain) &&
		   create_icv(this, assoc, plain, j, plain.ptr + plain.len);
}

METHOD(aead_t, decrypt, bool,
	private_gcm_aead_t *this, chunk_t encrypted, chunk_t assoc, chunk_t iv,
	chunk_t *plain)
{
	char j[BLOCK_SIZE];

	if (encrypted.len < this->icv_size)
	{
		return FALSE;
	}

	create_j(this, iv.ptr, j);

	encrypted.len -= this->icv_size;
	if (!verify_icv(this, assoc, encrypted, j, encrypted.ptr + encrypted.len))
	{
		return FALSE;
	}
	if (plain)
	{
		*plain = chunk_alloc(encrypted.len);
		return crypt(this, j, encrypted, *plain);
	}
	return crypt(this, j, encrypted, encrypted);
}

METHOD(aead_t, get_block_size, size_t,
	private_gcm_aead_t *this)
{
	return 1;
}

METHOD(aead_t, get_icv_size, size_t,
	private_gcm_aead_t *this)
{
	return this->icv_size;
}

METHOD(aead_t, get_iv_size, size_t,
	private_gcm_aead_t *this)
{
	return IV_SIZE;
}

METHOD(aead_t, get_iv_gen, iv_gen_t*,
	private_gcm_aead_t *this)
{
	return this->iv_gen;
}

METHOD(aead_t, get_key_size, size_t,
	private_gcm_aead_t *this)
{
	return this->crypter->get_key_size(this->crypter) + SALT_SIZE;
}

METHOD(aead_t, set_key, bool,
	private_gcm_aead_t *this, chunk_t key)
{
	memcpy(this->salt, key.ptr + key.len - SALT_SIZE, SALT_SIZE);
	key.len -= SALT_SIZE;
	return this->crypter->set_key(this->crypter, key) &&
		   create_h(this, this->h);
}

METHOD(aead_t, destroy, void,
	private_gcm_aead_t *this)
{
	this->crypter->destroy(this->crypter);
	this->iv_gen->destroy(this->iv_gen);
	free(this);
}

/**
 * See header
 */
gcm_aead_t *gcm_aead_create(encryption_algorithm_t algo,
							size_t key_size, size_t salt_size)
{
	private_gcm_aead_t *this;
	size_t icv_size;

	switch (key_size)
	{
		case 0:
			key_size = 16;
			break;
		case 16:
		case 24:
		case 32:
			break;
		default:
			return NULL;
	}
	if (salt_size && salt_size != SALT_SIZE)
	{
		/* currently not supported */
		return NULL;
	}
	switch (algo)
	{
		case ENCR_AES_GCM_ICV8:
			algo = ENCR_AES_CBC;
			icv_size = 8;
			break;
		case ENCR_AES_GCM_ICV12:
			algo = ENCR_AES_CBC;
			icv_size = 12;
			break;
		case ENCR_AES_GCM_ICV16:
			algo = ENCR_AES_CBC;
			icv_size = 16;
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.aead = {
				.encrypt = _encrypt,
				.decrypt = _decrypt,
				.get_block_size = _get_block_size,
				.get_icv_size = _get_icv_size,
				.get_iv_size = _get_iv_size,
				.get_iv_gen = _get_iv_gen,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.crypter = lib->crypto->create_crypter(lib->crypto, algo, key_size),
		.iv_gen = iv_gen_seq_create(),
		.icv_size = icv_size,
	);

	if (!this->crypter)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}
