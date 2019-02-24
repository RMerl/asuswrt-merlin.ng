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

#include "ccm_aead.h"

#include <crypto/iv/iv_gen_seq.h>

#define BLOCK_SIZE 16
#define SALT_SIZE 3
#define IV_SIZE 8
#define NONCE_SIZE (SALT_SIZE + IV_SIZE) /* 11 */
#define Q_SIZE (BLOCK_SIZE - NONCE_SIZE - 1) /* 4 */

typedef struct private_ccm_aead_t private_ccm_aead_t;

/**
 * Private data of an ccm_aead_t object.
 */
struct private_ccm_aead_t {

	/**
	 * Public ccm_aead_t interface.
	 */
	ccm_aead_t public;

	/**
	 * Underlying CBC crypter.
	 */
	crypter_t *crypter;

	/**
	 * IV generator.
	 */
	iv_gen_t *iv_gen;

	/**
	 * Length of the integrity check value
	 */
	size_t icv_size;

	/**
	 * salt to add to nonce
	 */
	u_char salt[SALT_SIZE];
};

/**
 * First block with control information
 */
typedef struct __attribute__((packed)) {
	BITFIELD4(uint8_t,
		/* size of p length field q, as q-1 */
		q_len: 3,
		/* size of our ICV t, as (t-2)/2 */
		t_len: 3,
		/* do we have associated data */
		assoc: 1,
		reserved: 1,
	) flags;
	/* nonce value */
	struct __attribute__((packed)) {
		u_char salt[SALT_SIZE];
		u_char iv[IV_SIZE];
	} nonce;
	/* length of plain text, q */
	u_char q[Q_SIZE];
} b0_t;

/**
 * Counter block
 */
typedef struct __attribute__((packed)) {
	BITFIELD3(uint8_t,
		/* size of p length field q, as q-1 */
		q_len: 3,
		zero: 3,
		reserved: 2,
	) flags;
	/* nonce value */
	struct __attribute__((packed)) {
		u_char salt[SALT_SIZE];
		u_char iv[IV_SIZE];
	} nonce;
	/* counter value */
	u_char i[Q_SIZE];
} ctr_t;

/**
 * Build the first block B0
 */
static void build_b0(private_ccm_aead_t *this, chunk_t plain, chunk_t assoc,
					 chunk_t iv, char *out)
{
	b0_t *block = (b0_t*)out;

	block->flags.reserved = 0;
	block->flags.assoc = assoc.len ? 1 : 0;
	block->flags.t_len = (this->icv_size - 2) / 2;
	block->flags.q_len = Q_SIZE - 1;
	memcpy(block->nonce.salt, this->salt, SALT_SIZE);
	memcpy(block->nonce.iv, iv.ptr, IV_SIZE);
	htoun32(block->q, plain.len);
}

/**
 * Build a counter block for counter i
 */
static void build_ctr(private_ccm_aead_t *this, uint32_t i, chunk_t iv,
					  char *out)
{
	ctr_t *ctr = (ctr_t*)out;

	ctr->flags.reserved = 0;
	ctr->flags.zero = 0;
	ctr->flags.q_len = Q_SIZE - 1;
	memcpy(ctr->nonce.salt, this->salt, SALT_SIZE);
	memcpy(ctr->nonce.iv, iv.ptr, IV_SIZE);
	htoun32(ctr->i, i);
}

/**
 * En-/Decrypt data
 */
static bool crypt_data(private_ccm_aead_t *this, chunk_t iv,
					   chunk_t in, chunk_t out)
{
	char ctr[BLOCK_SIZE];
	char zero[BLOCK_SIZE];
	char block[BLOCK_SIZE];

	build_ctr(this, 1, iv, ctr);
	memset(zero, 0, BLOCK_SIZE);

	while (in.len > 0)
	{
		memcpy(block, ctr, BLOCK_SIZE);
		if (!this->crypter->encrypt(this->crypter, chunk_from_thing(block),
									chunk_from_thing(zero), NULL))
		{
			return FALSE;
		}
		chunk_increment(chunk_from_thing(ctr));

		if (in.ptr != out.ptr)
		{
			memcpy(out.ptr, in.ptr, min(in.len, BLOCK_SIZE));
		}
		memxor(out.ptr, block, min(in.len, BLOCK_SIZE));
		in = chunk_skip(in, BLOCK_SIZE);
		out = chunk_skip(out, BLOCK_SIZE);
	}
	return TRUE;
}

/**
 * En-/Decrypt the ICV
 */
static bool crypt_icv(private_ccm_aead_t *this, chunk_t iv, char *icv)
{
	char ctr[BLOCK_SIZE];
	char zero[BLOCK_SIZE];

	build_ctr(this, 0, iv, ctr);
	memset(zero, 0, BLOCK_SIZE);

	if (!this->crypter->encrypt(this->crypter, chunk_from_thing(ctr),
								chunk_from_thing(zero), NULL))
	{
		return FALSE;
	}
	memxor(icv, ctr, this->icv_size);
	return TRUE;
}

/**
 * Create the ICV
 */
static bool create_icv(private_ccm_aead_t *this, chunk_t plain, chunk_t assoc,
					   chunk_t iv, char *icv)
{
	char zero[BLOCK_SIZE];
	chunk_t chunk;
	char *pos;
	int r, len;

	memset(zero, 0, BLOCK_SIZE);

	/* calculate number of blocks, including b0 */
	r = 1;
	if (assoc.len)
	{	/* assoc gets a 2 byte length header, gets padded to BLOCK_SIZE */
		r += (2 + assoc.len + BLOCK_SIZE - 1) / BLOCK_SIZE;
	}
	/* plain text gets padded to BLOCK_SIZE */
	r += (plain.len + BLOCK_SIZE - 1) / BLOCK_SIZE;

	/* concatenate data to a new chunk */
	chunk = chunk_alloc(r * BLOCK_SIZE);
	/* write control block */
	build_b0(this, plain, assoc, iv, chunk.ptr);
	pos = chunk.ptr + BLOCK_SIZE;
	/* append associated data, with length header */
	if (assoc.len)
	{
		/* currently we support two byte headers only (up to 2^16-2^8 bytes) */
		htoun16(pos, assoc.len);
		memcpy(pos + 2, assoc.ptr, assoc.len);
		pos += 2 + assoc.len;
		/* padding */
		len = (BLOCK_SIZE - ((2 + assoc.len) % BLOCK_SIZE)) % BLOCK_SIZE;
		memset(pos, 0, len);
		pos += len;
	}
	/* write plain data */
	memcpy(pos, plain.ptr, plain.len);
	pos += plain.len;
	/* padding */
	len = (BLOCK_SIZE - (plain.len % BLOCK_SIZE)) % BLOCK_SIZE;

	memset(pos, 0, len);

	/* encrypt inline with CBC, zero IV */
	if (!this->crypter->encrypt(this->crypter, chunk,
								chunk_from_thing(zero), NULL))
	{
		free(chunk.ptr);
		return FALSE;
	}
	/* copy last icv_size bytes as ICV to output */
	memcpy(icv, chunk.ptr + chunk.len - BLOCK_SIZE, this->icv_size);

	free(chunk.ptr);

	/* encrypt the ICV value */
	return crypt_icv(this, iv, icv);
}

/**
 * Verify the ICV
 */
static bool verify_icv(private_ccm_aead_t *this, chunk_t plain, chunk_t assoc,
					   chunk_t iv, char *icv)
{
	char buf[this->icv_size];

	return create_icv(this, plain, assoc, iv, buf) &&
		   memeq_const(buf, icv, this->icv_size);
}

METHOD(aead_t, encrypt, bool,
	private_ccm_aead_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
	chunk_t *encrypted)
{
	if (encrypted)
	{
		*encrypted = chunk_alloc(plain.len + this->icv_size);
		return create_icv(this, plain, assoc, iv, encrypted->ptr + plain.len) &&
			   crypt_data(this, iv, plain, *encrypted);
	}
	return create_icv(this, plain, assoc, iv, plain.ptr + plain.len) &&
		   crypt_data(this, iv, plain, plain);
}

METHOD(aead_t, decrypt, bool,
	private_ccm_aead_t *this, chunk_t encrypted, chunk_t assoc, chunk_t iv,
	chunk_t *plain)
{
	if (encrypted.len < this->icv_size)
	{
		return FALSE;
	}
	encrypted.len -= this->icv_size;
	if (plain)
	{
		*plain = chunk_alloc(encrypted.len);
		return crypt_data(this, iv, encrypted, *plain) &&
			   verify_icv(this, *plain, assoc, iv,
						  encrypted.ptr + encrypted.len);
	}
	return crypt_data(this, iv, encrypted, encrypted) &&
		   verify_icv(this, encrypted, assoc, iv,
					  encrypted.ptr + encrypted.len);
}

METHOD(aead_t, get_block_size, size_t,
	private_ccm_aead_t *this)
{
	return 1;
}

METHOD(aead_t, get_icv_size, size_t,
	private_ccm_aead_t *this)
{
	return this->icv_size;
}

METHOD(aead_t, get_iv_size, size_t,
	private_ccm_aead_t *this)
{
	return IV_SIZE;
}

METHOD(aead_t, get_iv_gen, iv_gen_t*,
	private_ccm_aead_t *this)
{
	return this->iv_gen;
}

METHOD(aead_t, get_key_size, size_t,
	private_ccm_aead_t *this)
{
	return this->crypter->get_key_size(this->crypter) + SALT_SIZE;
}

METHOD(aead_t, set_key, bool,
	private_ccm_aead_t *this, chunk_t key)
{
	memcpy(this->salt, key.ptr + key.len - SALT_SIZE, SALT_SIZE);
	key.len -= SALT_SIZE;
	return this->crypter->set_key(this->crypter, key);
}

METHOD(aead_t, destroy, void,
	private_ccm_aead_t *this)
{
	this->crypter->destroy(this->crypter);
	this->iv_gen->destroy(this->iv_gen);
	free(this);
}

/**
 * See header
 */
ccm_aead_t *ccm_aead_create(encryption_algorithm_t algo,
							size_t key_size, size_t salt_size)
{
	private_ccm_aead_t *this;
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
		case ENCR_AES_CCM_ICV8:
			algo = ENCR_AES_CBC;
			icv_size = 8;
			break;
		case ENCR_AES_CCM_ICV12:
			algo = ENCR_AES_CBC;
			icv_size = 12;
			break;
		case ENCR_AES_CCM_ICV16:
			algo = ENCR_AES_CBC;
			icv_size = 16;
			break;
		case ENCR_CAMELLIA_CCM_ICV8:
			algo = ENCR_CAMELLIA_CBC;
			icv_size = 8;
			break;
		case ENCR_CAMELLIA_CCM_ICV12:
			algo = ENCR_CAMELLIA_CBC;
			icv_size = 12;
			break;
		case ENCR_CAMELLIA_CCM_ICV16:
			algo = ENCR_CAMELLIA_CBC;
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
