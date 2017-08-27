/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
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

#include "aead.h"

#include <utils/debug.h>
#include <crypto/iv/iv_gen_rand.h>

typedef struct private_aead_t private_aead_t;

/**
 * Private data of an aead_t object.
 */
struct private_aead_t {

	/**
	 * Public aead_t interface.
	 */
	aead_t public;

	/**
	 * traditional crypter
	 */
	crypter_t *crypter;

	/**
	 * traditional signer
	 */
	signer_t *signer;

	/**
	 * IV generator
	 */
	iv_gen_t *iv_gen;
};

METHOD(aead_t, encrypt, bool,
	private_aead_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
	chunk_t *encrypted)
{
	chunk_t encr, sig;

	if (!this->signer->get_signature(this->signer, assoc, NULL) ||
		!this->signer->get_signature(this->signer, iv, NULL))
	{
		return FALSE;
	}

	if (encrypted)
	{
		if (!this->crypter->encrypt(this->crypter, plain, iv, &encr))
		{
			return FALSE;
		}
		if (!this->signer->allocate_signature(this->signer, encr, &sig))
		{
			free(encr.ptr);
			return FALSE;
		}
		*encrypted = chunk_cat("cmm", iv, encr, sig);
	}
	else
	{
		if (!this->crypter->encrypt(this->crypter, plain, iv, NULL) ||
			!this->signer->get_signature(this->signer,
										 plain, plain.ptr + plain.len))
		{
			return FALSE;
		}
	}
	return TRUE;
}

METHOD(aead_t, decrypt, bool,
	private_aead_t *this, chunk_t encrypted, chunk_t assoc, chunk_t iv,
	chunk_t *plain)
{
	chunk_t sig;
	size_t bs;

	bs = this->crypter->get_block_size(this->crypter);
	sig.len = this->signer->get_block_size(this->signer);
	if (sig.len > encrypted.len || (encrypted.len - sig.len) % bs)
	{
		DBG1(DBG_LIB, "invalid encrypted data length %d with block size %d",
			encrypted.len - sig.len, bs);
		return FALSE;
	}
	chunk_split(encrypted, "mm", encrypted.len - sig.len,
				&encrypted, sig.len, &sig);

	if (!this->signer->get_signature(this->signer, assoc, NULL) ||
		!this->signer->get_signature(this->signer, iv, NULL))
	{
		return FALSE;
	}
	if (!this->signer->verify_signature(this->signer, encrypted, sig))
	{
		DBG1(DBG_LIB, "MAC verification failed");
		return FALSE;
	}
	return this->crypter->decrypt(this->crypter, encrypted, iv, plain);
}

METHOD(aead_t, get_block_size, size_t,
	private_aead_t *this)
{
	return this->crypter->get_block_size(this->crypter);
}

METHOD(aead_t, get_icv_size, size_t,
	private_aead_t *this)
{
	return this->signer->get_block_size(this->signer);
}

METHOD(aead_t, get_iv_size, size_t,
	private_aead_t *this)
{
	return this->crypter->get_iv_size(this->crypter);
}

METHOD(aead_t, get_iv_gen, iv_gen_t*,
	private_aead_t *this)
{
	return this->iv_gen;
}

METHOD(aead_t, get_key_size, size_t,
	private_aead_t *this)
{
	return this->crypter->get_key_size(this->crypter) +
			this->signer->get_key_size(this->signer);
}

METHOD(aead_t, set_key, bool,
	private_aead_t *this, chunk_t key)
{
	chunk_t sig, enc;

	chunk_split(key, "mm", this->signer->get_key_size(this->signer), &sig,
				this->crypter->get_key_size(this->crypter), &enc);

	return this->signer->set_key(this->signer, sig) &&
		   this->crypter->set_key(this->crypter, enc);
}

METHOD(aead_t, destroy, void,
	private_aead_t *this)
{
	this->iv_gen->destroy(this->iv_gen);
	this->crypter->destroy(this->crypter);
	this->signer->destroy(this->signer);
	free(this);
}

/**
 * See header
 */
aead_t *aead_create(crypter_t *crypter, signer_t *signer)
{
	private_aead_t *this;

	INIT(this,
		.public = {
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
		.crypter = crypter,
		.signer = signer,
		.iv_gen = iv_gen_rand_create(),
	);

	return &this->public;
}
