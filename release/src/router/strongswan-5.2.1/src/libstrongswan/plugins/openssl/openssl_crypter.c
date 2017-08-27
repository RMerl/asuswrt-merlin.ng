/*
 * Copyright (C) 2008 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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

#include "openssl_crypter.h"

#include <openssl/evp.h>

typedef struct private_openssl_crypter_t private_openssl_crypter_t;

/**
 * Private data of openssl_crypter_t
 */
struct private_openssl_crypter_t {

	/**
	 * Public part of this class.
	 */
	openssl_crypter_t public;

	/*
	 * the key
	 */
	chunk_t	key;

	/*
	 * the cipher to use
	 */
	const EVP_CIPHER *cipher;
};

/**
 * Look up an OpenSSL algorithm name and validate its key size
 */
static char* lookup_algorithm(u_int16_t ikev2_algo, size_t *key_size)
{
	struct {
		/* identifier specified in IKEv2 */
		int ikev2_id;
		/* name of the algorithm, as used in OpenSSL */
		char *name;
		/* default key size in bytes */
		size_t key_def;
		/* minimum key size */
		size_t key_min;
		/* maximum key size */
		size_t key_max;
	} mappings[] = {
		{ENCR_DES, 			"des",			 8,		 8,		  8},
		{ENCR_3DES, 		"des3",			24,		24,		 24},
		{ENCR_RC5, 			"rc5", 			16,		 5,		255},
		{ENCR_IDEA, 		"idea",			16,		16,		 16},
		{ENCR_CAST, 		"cast",			16,		 5,		 16},
		{ENCR_BLOWFISH, 	"blowfish",		16,		 5,		 56},
	};
	int i;

	for (i = 0; i < countof(mappings); i++)
	{
		if (ikev2_algo == mappings[i].ikev2_id)
		{
			/* set the key size if it is not set */
			if (*key_size == 0)
			{
				*key_size = mappings[i].key_def;
			}
			/* validate key size */
			if (*key_size < mappings[i].key_min ||
				*key_size > mappings[i].key_max)
			{
				return NULL;
			}
			return mappings[i].name;
		}
	}
	return NULL;
}

/**
 * Do the actual en/decryption in an EVP context
 */
static bool crypt(private_openssl_crypter_t *this, chunk_t data, chunk_t iv,
				  chunk_t *dst, int enc)
{
	int len;
	u_char *out;

	out = data.ptr;
	if (dst)
	{
		*dst = chunk_alloc(data.len);
		out = dst->ptr;
	}
	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);
	return EVP_CipherInit_ex(&ctx, this->cipher, NULL, NULL, NULL, enc) &&
		   EVP_CIPHER_CTX_set_padding(&ctx, 0) /* disable padding */ &&
		   EVP_CIPHER_CTX_set_key_length(&ctx, this->key.len) &&
		   EVP_CipherInit_ex(&ctx, NULL, NULL, this->key.ptr, iv.ptr, enc) &&
		   EVP_CipherUpdate(&ctx, out, &len, data.ptr, data.len) &&
		   /* since padding is disabled this does nothing */
		   EVP_CipherFinal_ex(&ctx, out + len, &len) &&
		   EVP_CIPHER_CTX_cleanup(&ctx);
}

METHOD(crypter_t, decrypt, bool,
	private_openssl_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	return crypt(this, data, iv, dst, 0);
}

METHOD(crypter_t, encrypt, bool,
	private_openssl_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	return crypt(this, data, iv, dst, 1);
}

METHOD(crypter_t, get_block_size, size_t,
	private_openssl_crypter_t *this)
{
	return this->cipher->block_size;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_openssl_crypter_t *this)
{
	return this->cipher->block_size;
}

METHOD(crypter_t, get_key_size, size_t,
	private_openssl_crypter_t *this)
{
	return this->key.len;
}

METHOD(crypter_t, set_key, bool,
	private_openssl_crypter_t *this, chunk_t key)
{
	memcpy(this->key.ptr, key.ptr, min(key.len, this->key.len));
	return TRUE;
}

METHOD(crypter_t, destroy, void,
	private_openssl_crypter_t *this)
{
	chunk_clear(&this->key);
	free(this);
}

/*
 * Described in header
 */
openssl_crypter_t *openssl_crypter_create(encryption_algorithm_t algo,
												  size_t key_size)
{
	private_openssl_crypter_t *this;

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
	);

	switch (algo)
	{
		case ENCR_NULL:
			this->cipher = EVP_enc_null();
			key_size = 0;
			break;
		case ENCR_AES_CBC:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:        /* AES 128 */
					this->cipher = EVP_get_cipherbyname("aes128");
					break;
				case 24:        /* AES-192 */
					this->cipher = EVP_get_cipherbyname("aes192");
					break;
				case 32:        /* AES-256 */
					this->cipher = EVP_get_cipherbyname("aes256");
					break;
				default:
					free(this);
					return NULL;
			}
			break;
		case ENCR_CAMELLIA_CBC:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:        /* CAMELLIA 128 */
					this->cipher = EVP_get_cipherbyname("camellia128");
					break;
				case 24:        /* CAMELLIA 192 */
					this->cipher = EVP_get_cipherbyname("camellia192");
					break;
				case 32:        /* CAMELLIA 256 */
					this->cipher = EVP_get_cipherbyname("camellia256");
					break;
				default:
					free(this);
					return NULL;
			}
			break;
		case ENCR_DES_ECB:
			key_size = 8;
			this->cipher = EVP_des_ecb();
			break;
		default:
		{
			char* name;

			name = lookup_algorithm(algo, &key_size);
			if (!name)
			{
				/* algo unavailable or key_size invalid */
				free(this);
				return NULL;
			}
			this->cipher = EVP_get_cipherbyname(name);
			break;
		}
	}

	if (!this->cipher)
	{
		/* OpenSSL does not support the requested algo */
		free(this);
		return NULL;
	}

	this->key = chunk_alloc(key_size);

	return &this->public;
}
