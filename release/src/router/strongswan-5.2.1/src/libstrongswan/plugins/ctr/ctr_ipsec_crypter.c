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

#include "ctr_ipsec_crypter.h"

typedef struct private_ctr_ipsec_crypter_t private_ctr_ipsec_crypter_t;

/**
 * Private data of an ctr_ipsec_crypter_t object.
 */
struct private_ctr_ipsec_crypter_t {

	/**
	 * Public ctr_ipsec_crypter_t interface.
	 */
	ctr_ipsec_crypter_t public;

	/**
	 * Underlying CBC crypter
	 */
	crypter_t *crypter;

	/**
	 * counter state
	 */
	struct {
		char nonce[4];
		char iv[8];
		u_int32_t counter;
	} __attribute__((packed)) state;
};

/**
 * Do the CTR crypto operation
 */
static bool crypt_ctr(private_ctr_ipsec_crypter_t *this,
					  chunk_t in, chunk_t out)
{
	size_t is, bs;
	chunk_t state;

	is = this->crypter->get_iv_size(this->crypter);
	bs = sizeof(this->state);

	this->state.counter = htonl(1);
	state = chunk_create((char*)&this->state, bs);

	while (in.len > 0)
	{
		char iv[is], block[bs];

		memset(iv, 0, is);
		memcpy(block, state.ptr, bs);
		if (!this->crypter->encrypt(this->crypter, chunk_create(block, bs),
									chunk_create(iv, is), NULL))
		{
			return FALSE;
		}
		chunk_increment(state);

		if (in.ptr != out.ptr)
		{
			memcpy(out.ptr, in.ptr, min(in.len, bs));
		}
		memxor(out.ptr, block, min(in.len, bs));
		in = chunk_skip(in, bs);
		out = chunk_skip(out, bs);
	}
	return TRUE;
}

METHOD(crypter_t, crypt, bool,
	private_ctr_ipsec_crypter_t *this, chunk_t in, chunk_t iv, chunk_t *out)
{
	memcpy(this->state.iv, iv.ptr, sizeof(this->state.iv));

	if (out)
	{
		*out = chunk_alloc(in.len);
		return crypt_ctr(this, in, *out);
	}
	return crypt_ctr(this, in, in);
}

METHOD(crypter_t, get_block_size, size_t,
	private_ctr_ipsec_crypter_t *this)
{
	return 1;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_ctr_ipsec_crypter_t *this)
{
	return sizeof(this->state.iv);
}

METHOD(crypter_t, get_key_size, size_t,
	private_ctr_ipsec_crypter_t *this)
{
	return this->crypter->get_key_size(this->crypter)
			+ sizeof(this->state.nonce);
}

METHOD(crypter_t, set_key, bool,
	private_ctr_ipsec_crypter_t *this, chunk_t key)
{
	memcpy(this->state.nonce, key.ptr + key.len - sizeof(this->state.nonce),
		   sizeof(this->state.nonce));
	key.len -= sizeof(this->state.nonce);
	return this->crypter->set_key(this->crypter, key);
}

METHOD(crypter_t, destroy, void,
	private_ctr_ipsec_crypter_t *this)
{
	this->crypter->destroy(this->crypter);
	free(this);
}

/**
 * See header
 */
ctr_ipsec_crypter_t *ctr_ipsec_crypter_create(encryption_algorithm_t algo,
											  size_t key_size)
{
	private_ctr_ipsec_crypter_t *this;

	switch (algo)
	{
		case ENCR_AES_CTR:
			algo = ENCR_AES_CBC;
			break;
		case ENCR_CAMELLIA_CTR:
			algo = ENCR_CAMELLIA_CBC;
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.crypter = {
				.encrypt = _crypt,
				.decrypt = _crypt,
				.get_block_size = _get_block_size,
				.get_iv_size = _get_iv_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.crypter = lib->crypto->create_crypter(lib->crypto, algo, key_size),
	);

	if (!this->crypter)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}
