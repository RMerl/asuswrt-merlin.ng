/*
 * Copyright (C) 2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#include "gcrypt_crypter.h"

#include <gcrypt.h>

#include <utils/debug.h>

typedef struct private_gcrypt_crypter_t private_gcrypt_crypter_t;

/**
 * Private data of gcrypt_crypter_t
 */
struct private_gcrypt_crypter_t {

	/**
	 * Public part of this class.
	 */
	gcrypt_crypter_t public;

	/**
	 * gcrypt cipher handle
	 */
	gcry_cipher_hd_t h;

	/**
	 * gcrypt algorithm identifier
	 */
	int alg;

	/**
	 * are we using counter mode?
	 */
	bool ctr_mode;

	/**
	 * counter state
	 */
	struct {
		char nonce[4];
		char iv[8];
		uint32_t counter;
	} __attribute__((packed)) ctr;
};

/**
 * Set the IV for en/decryption
 */
static bool set_iv(private_gcrypt_crypter_t *this, chunk_t iv)
{
	if (this->ctr_mode)
	{
		memcpy(this->ctr.iv, iv.ptr, sizeof(this->ctr.iv));
		this->ctr.counter = htonl(1);
		return gcry_cipher_setctr(this->h, &this->ctr, sizeof(this->ctr)) == 0;
	}
	return gcry_cipher_setiv(this->h, iv.ptr, iv.len) == 0;
}

METHOD(crypter_t, decrypt, bool,
	private_gcrypt_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	if (!set_iv(this, iv))
	{
		return FALSE;
	}
	if (dst)
	{
		*dst = chunk_alloc(data.len);
		return gcry_cipher_decrypt(this->h, dst->ptr, dst->len,
								   data.ptr, data.len) == 0;
	}
	return gcry_cipher_decrypt(this->h, data.ptr, data.len, NULL, 0) == 0;
}

METHOD(crypter_t, encrypt, bool,
	private_gcrypt_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	if (!set_iv(this, iv))
	{
		return FALSE;
	}
	if (dst)
	{
		*dst = chunk_alloc(data.len);
		return gcry_cipher_encrypt(this->h, dst->ptr, dst->len,
								   data.ptr, data.len) == 0;
	}
	return gcry_cipher_encrypt(this->h, data.ptr, data.len, NULL, 0) == 0;
}

METHOD(crypter_t, get_block_size, size_t,
	private_gcrypt_crypter_t *this)
{
	size_t len = 0;

	if (this->ctr_mode)
	{	/* counter mode does not need any padding */
		return 1;
	}
	gcry_cipher_algo_info(this->alg, GCRYCTL_GET_BLKLEN, NULL, &len);
	return len;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_gcrypt_crypter_t *this)
{
	size_t len = 0;

	if (this->ctr_mode)
	{
		return sizeof(this->ctr.iv);
	}
	gcry_cipher_algo_info(this->alg, GCRYCTL_GET_BLKLEN, NULL, &len);
	return len;
}

METHOD(crypter_t, get_key_size, size_t,
	private_gcrypt_crypter_t *this)
{
	size_t len = 0;

	gcry_cipher_algo_info(this->alg, GCRYCTL_GET_KEYLEN, NULL, &len);
	if (this->ctr_mode)
	{
		return len + sizeof(this->ctr.nonce);
	}
	return len;
}

METHOD(crypter_t, set_key, bool,
	private_gcrypt_crypter_t *this, chunk_t key)
{
	if (this->ctr_mode)
	{
		/* last 4 bytes are the nonce */
		memcpy(this->ctr.nonce, key.ptr + key.len - sizeof(this->ctr.nonce),
			   sizeof(this->ctr.nonce));
		key.len -= sizeof(this->ctr.nonce);
	}
	return gcry_cipher_setkey(this->h, key.ptr, key.len) == 0;
}

METHOD(crypter_t, destroy, void,
	private_gcrypt_crypter_t *this)
{
	gcry_cipher_close(this->h);
	free(this);
}

/*
 * Described in header
 */
gcrypt_crypter_t *gcrypt_crypter_create(encryption_algorithm_t algo,
										size_t key_size)
{
	private_gcrypt_crypter_t *this;
	int gcrypt_alg;
	int mode = GCRY_CIPHER_MODE_CBC;
	gcry_error_t err;

	switch (algo)
	{
		case ENCR_DES:
			gcrypt_alg = GCRY_CIPHER_DES;
			break;
		case ENCR_DES_ECB:
			gcrypt_alg = GCRY_CIPHER_DES;
			mode = GCRY_CIPHER_MODE_ECB;
			break;
		case ENCR_3DES:
			gcrypt_alg = GCRY_CIPHER_3DES;
			break;
		case ENCR_IDEA:
			/* currently not implemented in gcrypt */
			return NULL;
		case ENCR_CAST:
			gcrypt_alg = GCRY_CIPHER_CAST5;
			break;
		case ENCR_BLOWFISH:
			if (key_size != 16 && key_size != 0)
			{	/* gcrypt currently supports 128 bit blowfish only */
				return NULL;
			}
			gcrypt_alg = GCRY_CIPHER_BLOWFISH;
			break;
		case ENCR_AES_CTR:
			mode = GCRY_CIPHER_MODE_CTR;
			/* fall */
		case ENCR_AES_CBC:
			switch (key_size)
			{
				case 0:
				case 16:
					gcrypt_alg = GCRY_CIPHER_AES128;
					break;
				case 24:
					gcrypt_alg = GCRY_CIPHER_AES192;
					break;
				case 32:
					gcrypt_alg = GCRY_CIPHER_AES256;
					break;
				default:
					return NULL;
			}
			break;
		case ENCR_CAMELLIA_CTR:
			mode = GCRY_CIPHER_MODE_CTR;
			/* fall */
		case ENCR_CAMELLIA_CBC:
			switch (key_size)
			{
#ifdef HAVE_GCRY_CIPHER_CAMELLIA
				case 0:
				case 16:
					gcrypt_alg = GCRY_CIPHER_CAMELLIA128;
					break;
				case 24:
					gcrypt_alg = GCRY_CIPHER_CAMELLIA192;
					break;
				case 32:
					gcrypt_alg = GCRY_CIPHER_CAMELLIA256;
					break;
#endif /* HAVE_GCRY_CIPHER_CAMELLIA */
				default:
					return NULL;
			}
			break;
		case ENCR_SERPENT_CBC:
			switch (key_size)
			{
				case 0:
				case 16:
					gcrypt_alg = GCRY_CIPHER_SERPENT128;
					break;
				case 24:
					gcrypt_alg = GCRY_CIPHER_SERPENT192;
					break;
				case 32:
					gcrypt_alg = GCRY_CIPHER_SERPENT256;
					break;
				default:
					return NULL;
			}
			break;
		case ENCR_TWOFISH_CBC:
			switch (key_size)
			{
				case 0:
				case 16:
					gcrypt_alg = GCRY_CIPHER_TWOFISH128;
					break;
				case 32:
					gcrypt_alg = GCRY_CIPHER_TWOFISH;
					break;
				default:
					return NULL;
			}
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.crypter = {
				.encrypt = _encrypt,
				.decrypt = _decrypt,
				.get_block_size = _get_block_size,
				.get_iv_size = _get_iv_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.alg = gcrypt_alg,
		.ctr_mode = mode == GCRY_CIPHER_MODE_CTR,
	);

	err = gcry_cipher_open(&this->h, gcrypt_alg, mode, 0);
	if (err)
	{
		DBG1(DBG_LIB, "grcy_cipher_open(%N) failed: %s",
			 encryption_algorithm_names, algo, gpg_strerror(err));
		free(this);
		return NULL;
	}
	return &this->public;
}

