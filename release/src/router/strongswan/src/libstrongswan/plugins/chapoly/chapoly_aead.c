/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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

#include "chapoly_aead.h"
#include "chapoly_drv.h"

#include <crypto/iv/iv_gen_seq.h>

/* maximum plain message size */
#define P_MAX 247877906880

typedef struct private_chapoly_aead_t private_chapoly_aead_t;

/**
 * Private data of an chapoly_aead_t object.
 */
struct private_chapoly_aead_t {

	/**
	 * Public chapoly_aead_t interface.
	 */
	chapoly_aead_t public;

	/**
	 * IV generator.
	 */
	iv_gen_t *iv_gen;

	/**
	 * Driver backend
	 */
	chapoly_drv_t *drv;
};

/**
 * Include a partial block to ICV by padding it with zero bytes
 */
static bool poly_update_padded(private_chapoly_aead_t *this,
							   u_char *in, size_t len)
{
	u_char b[POLY_BLOCK_SIZE];

	memset(b, 0, sizeof(b));
	memcpy(b, in, len);

	return this->drv->poly(this->drv, b, 1);
}

/**
 * Include associated data with padding to ICV
 */
static bool poly_head(private_chapoly_aead_t *this, u_char *assoc, size_t len)
{
	u_int blocks, rem;

	blocks = len / POLY_BLOCK_SIZE;
	rem = len % POLY_BLOCK_SIZE;
	if (!this->drv->poly(this->drv, assoc, blocks))
	{
		return FALSE;
	}
	if (rem)
	{
		return poly_update_padded(this, assoc + blocks * POLY_BLOCK_SIZE, rem);
	}
	return TRUE;
}

/**
 * Include length fields to ICV
 */
static bool poly_tail(private_chapoly_aead_t *this, size_t alen, size_t clen)
{
	struct {
		uint64_t alen;
		uint64_t clen;
	} b;

	b.alen = htole64(alen);
	b.clen = htole64(clen);

	return this->drv->poly(this->drv, (u_char*)&b, 1);
}

/**
 * Perform ChaCha20 encryption inline and generate an ICV tag
 */
static bool do_encrypt(private_chapoly_aead_t *this, size_t len, u_char *data,
					   u_char *iv, size_t alen, u_char *assoc, u_char *icv)
{
	u_int blocks, rem, prem;

	if (!this->drv->init(this->drv, iv) ||
		!poly_head(this, assoc, alen))
	{
		return FALSE;
	}
	blocks = len / CHACHA_BLOCK_SIZE;
	if (!this->drv->encrypt(this->drv, data, blocks))
	{
		return FALSE;
	}
	rem = len % CHACHA_BLOCK_SIZE;
	if (rem)
	{
		u_char stream[CHACHA_BLOCK_SIZE];

		data += blocks * CHACHA_BLOCK_SIZE;
		if (!this->drv->chacha(this->drv, stream))
		{
			return FALSE;
		}
		memxor(data, stream, rem);

		blocks = rem / POLY_BLOCK_SIZE;
		if (!this->drv->poly(this->drv, data, blocks))
		{
			return FALSE;
		}
		prem = rem % POLY_BLOCK_SIZE;
		if (prem)
		{
			poly_update_padded(this, data + blocks * POLY_BLOCK_SIZE, prem);
		}
	}
	return poly_tail(this, alen, len) &&
		   this->drv->finish(this->drv, icv);
}

/**
 * Perform ChaCha20 decryption inline and generate an ICV tag
 */
static bool do_decrypt(private_chapoly_aead_t *this, size_t len, u_char *data,
					   u_char *iv, size_t alen, u_char *assoc, u_char *icv)
{
	u_int blocks, rem, prem;

	if (!this->drv->init(this->drv, iv) ||
		!poly_head(this, assoc, alen))
	{
		return FALSE;
	}
	blocks = len / CHACHA_BLOCK_SIZE;
	if (!this->drv->decrypt(this->drv, data, blocks))
	{
		return FALSE;
	}
	rem = len % CHACHA_BLOCK_SIZE;
	if (rem)
	{
		u_char stream[CHACHA_BLOCK_SIZE];

		data += blocks * CHACHA_BLOCK_SIZE;

		blocks = rem / POLY_BLOCK_SIZE;
		if (!this->drv->poly(this->drv, data, blocks))
		{
			return FALSE;
		}
		prem = rem % POLY_BLOCK_SIZE;
		if (prem)
		{
			poly_update_padded(this, data + blocks * POLY_BLOCK_SIZE, prem);
		}
		if (!this->drv->chacha(this->drv, stream))
		{
			return FALSE;
		}
		memxor(data, stream, rem);
	}
	return poly_tail(this, alen, len) &&
		   this->drv->finish(this->drv, icv);
}

METHOD(aead_t, encrypt, bool,
	private_chapoly_aead_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
	chunk_t *encr)
{
	u_char *out;

	if (sizeof(plain.len) > sizeof(uint32_t) && plain.len > P_MAX)
	{
		return FALSE;
	}
	if (iv.len != CHACHA_IV_SIZE)
	{
		return FALSE;
	}
	out = plain.ptr;
	if (encr)
	{
		*encr = chunk_alloc(plain.len + POLY_ICV_SIZE);
		out = encr->ptr;
		memcpy(out, plain.ptr, plain.len);
	}
	do_encrypt(this, plain.len, out, iv.ptr, assoc.len, assoc.ptr,
			   out + plain.len);
	return TRUE;
}

METHOD(aead_t, decrypt, bool,
	private_chapoly_aead_t *this, chunk_t encr, chunk_t assoc, chunk_t iv,
	chunk_t *plain)
{
	u_char *out, icv[POLY_ICV_SIZE];
	if (iv.len != CHACHA_IV_SIZE || encr.len < POLY_ICV_SIZE)
	{
		return FALSE;
	}
	encr.len -= POLY_ICV_SIZE;
	if (sizeof(encr.len) > sizeof(uint32_t) && encr.len > P_MAX)
	{
		return FALSE;
	}
	out = encr.ptr;
	if (plain)
	{
		*plain = chunk_alloc(encr.len);
		out = plain->ptr;
		memcpy(out, encr.ptr, encr.len);
	}
	do_decrypt(this, encr.len, out, iv.ptr, assoc.len, assoc.ptr, icv);
	return memeq_const(icv, encr.ptr + encr.len, POLY_ICV_SIZE);
}

METHOD(aead_t, get_block_size, size_t,
	private_chapoly_aead_t *this)
{
	return 1;
}

METHOD(aead_t, get_icv_size, size_t,
	private_chapoly_aead_t *this)
{
	return POLY_ICV_SIZE;
}

METHOD(aead_t, get_iv_size, size_t,
	private_chapoly_aead_t *this)
{
	return CHACHA_IV_SIZE;
}

METHOD(aead_t, get_iv_gen, iv_gen_t*,
	private_chapoly_aead_t *this)
{
	return this->iv_gen;
}

METHOD(aead_t, get_key_size, size_t,
	private_chapoly_aead_t *this)
{
	return CHACHA_KEY_SIZE + CHACHA_SALT_SIZE;
}

METHOD(aead_t, set_key, bool,
	private_chapoly_aead_t *this, chunk_t key)
{
	if (key.len != CHACHA_KEY_SIZE + CHACHA_SALT_SIZE)
	{
		return FALSE;
	}
	return this->drv->set_key(this->drv, "expand 32-byte k",
							  key.ptr, key.ptr + CHACHA_KEY_SIZE);
}

METHOD(aead_t, destroy, void,
	private_chapoly_aead_t *this)
{
	this->drv->destroy(this->drv);
	this->iv_gen->destroy(this->iv_gen);
	free(this);
}

/**
 * See header
 */
chapoly_aead_t *chapoly_aead_create(encryption_algorithm_t algo,
									size_t key_size, size_t salt_size)
{
	private_chapoly_aead_t *this;
	chapoly_drv_t *drv;

	if (algo != ENCR_CHACHA20_POLY1305)
	{
		return NULL;
	}
	if (key_size && key_size != CHACHA_KEY_SIZE)
	{
		return NULL;
	}
	if (salt_size && salt_size != CHACHA_SALT_SIZE)
	{
		return NULL;
	}
	drv = chapoly_drv_probe();
	if (!drv)
	{
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
		.iv_gen = iv_gen_seq_create(),
		.drv = drv,
	);

	return &this->public;
}
