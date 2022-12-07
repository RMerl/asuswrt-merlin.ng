/*
 * Copyright (C) 2008-2022 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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
 * These are as defined by RFC 3686
 */
#define CTR_NONCE_LEN	4
#define CTR_IV_LEN		8

/**
 * Private data of openssl_crypter_t
 */
struct private_openssl_crypter_t {

	/**
	 * Public part of this class.
	 */
	openssl_crypter_t public;

	/*
	 * The key
	 */
	chunk_t	key;

	/**
	 * Nonce value (CTR mode)
	 */
	chunk_t nonce;

	/*
	 * The cipher to use
	 */
	const EVP_CIPHER *cipher;
};

/**
 * Look up an OpenSSL algorithm name and validate its key size
 */
static char* lookup_algorithm(uint16_t ikev2_algo, size_t *key_size)
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
		{ENCR_DES, 			"des-cbc",		 8,		 8,		  8},
		{ENCR_3DES, 		"des-ede3-cbc",	24,		24,		 24},
		{ENCR_RC5, 			"rc5-cbc",		16,		 5,		255},
		{ENCR_IDEA, 		"idea-cbc",		16,		16,		 16},
		{ENCR_CAST, 		"cast5-cbc",	16,		 5,		 16},
		{ENCR_BLOWFISH, 	"bf-cbc",		16,		 5,		 56},
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
	EVP_CIPHER_CTX *ctx;
	int len;
	u_char iv_buf[EVP_CIPHER_iv_length(this->cipher)], *iv_ptr = iv_buf, *out;
	bool success = FALSE;

	if (this->nonce.len && (this->nonce.len + iv.len) <= sizeof(iv_buf))
	{
		memset(iv_buf, 0, sizeof(iv_buf));
		memcpy(iv_buf, this->nonce.ptr, this->nonce.len);
		memcpy(iv_buf + this->nonce.len, iv.ptr, iv.len);
		iv_buf[sizeof(iv_buf) - 1] = 1;
	}
	else if (iv.len == sizeof(iv_buf))
	{
		iv_ptr = iv.ptr;
	}
	else
	{
		return FALSE;
	}
	out = data.ptr;
	if (dst)
	{
		*dst = chunk_alloc(data.len);
		out = dst->ptr;
	}
	ctx = EVP_CIPHER_CTX_new();
	if (EVP_CipherInit_ex(ctx, this->cipher, NULL, NULL, NULL, enc) &&
		EVP_CIPHER_CTX_set_padding(ctx, 0) /* disable padding */ &&
		EVP_CIPHER_CTX_set_key_length(ctx, this->key.len) &&
		EVP_CipherInit_ex(ctx, NULL, NULL, this->key.ptr, iv_ptr, enc) &&
		EVP_CipherUpdate(ctx, out, &len, data.ptr, data.len) &&
		/* since padding is disabled this does nothing */
		EVP_CipherFinal_ex(ctx, out + len, &len))
	{
		success = TRUE;
	}
	EVP_CIPHER_CTX_free(ctx);
	return success;
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
	return EVP_CIPHER_block_size(this->cipher);
}

METHOD(crypter_t, get_iv_size, size_t,
	private_openssl_crypter_t *this)
{
	if (this->nonce.len)
	{
		return CTR_IV_LEN;
	}
	return EVP_CIPHER_iv_length(this->cipher);
}

METHOD(crypter_t, get_key_size, size_t,
	private_openssl_crypter_t *this)
{
	return this->key.len + this->nonce.len;
}

METHOD(crypter_t, set_key, bool,
	private_openssl_crypter_t *this, chunk_t key)
{
	if (key.len != get_key_size(this))
	{
		return FALSE;
	}
	memcpy(this->nonce.ptr, key.ptr + key.len - this->nonce.len, this->nonce.len);
	memcpy(this->key.ptr, key.ptr, this->key.len);
	return TRUE;
}

METHOD(crypter_t, destroy, void,
	private_openssl_crypter_t *this)
{
	chunk_clear(&this->key);
	chunk_clear(&this->nonce);
	free(this);
}

/*
 * Described in header
 */
openssl_crypter_t *openssl_crypter_create(encryption_algorithm_t algo,
												  size_t key_size)
{
	private_openssl_crypter_t *this;
	size_t nonce_size = 0;

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
					this->cipher = EVP_get_cipherbyname("aes-128-cbc");
					break;
				case 24:        /* AES-192 */
					this->cipher = EVP_get_cipherbyname("aes-192-cbc");
					break;
				case 32:        /* AES-256 */
					this->cipher = EVP_get_cipherbyname("aes-256-cbc");
					break;
				default:
					free(this);
					return NULL;
			}
			break;
		case ENCR_AES_CTR:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:        /* AES 128 */
					this->cipher = EVP_get_cipherbyname("aes-128-ctr");
					break;
				case 24:        /* AES-192 */
					this->cipher = EVP_get_cipherbyname("aes-192-ctr");
					break;
				case 32:        /* AES-256 */
					this->cipher = EVP_get_cipherbyname("aes-256-ctr");
					break;
				default:
					free(this);
					return NULL;
			}
			nonce_size = CTR_NONCE_LEN;
			break;
		case ENCR_AES_ECB:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:        /* AES 128 */
					this->cipher = EVP_get_cipherbyname("aes-128-ecb");
					break;
				case 24:        /* AES-192 */
					this->cipher = EVP_get_cipherbyname("aes-192-ecb");
					break;
				case 32:        /* AES-256 */
					this->cipher = EVP_get_cipherbyname("aes-256-ecb");
					break;
				default:
					free(this);
					return NULL;
			}
			break;
		case ENCR_AES_CFB:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:        /* AES 128 */
					this->cipher = EVP_get_cipherbyname("aes-128-cfb");
					break;
				case 24:        /* AES-192 */
					this->cipher = EVP_get_cipherbyname("aes-192-cfb");
					break;
				case 32:        /* AES-256 */
					this->cipher = EVP_get_cipherbyname("aes-256-cfb");
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
					this->cipher = EVP_get_cipherbyname("camellia-128-cbc");
					break;
				case 24:        /* CAMELLIA 192 */
					this->cipher = EVP_get_cipherbyname("camellia-192-cbc");
					break;
				case 32:        /* CAMELLIA 256 */
					this->cipher = EVP_get_cipherbyname("camellia-256-cbc");
					break;
				default:
					free(this);
					return NULL;
			}
			break;
		case ENCR_CAMELLIA_CTR:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:        /* CAMELLIA 128 */
					this->cipher = EVP_get_cipherbyname("camellia-128-ctr");
					break;
				case 24:        /* CAMELLIA 192 */
					this->cipher = EVP_get_cipherbyname("camellia-192-ctr");
					break;
				case 32:        /* CAMELLIA 256 */
					this->cipher = EVP_get_cipherbyname("camellia-256-ctr");
					break;
				default:
					free(this);
					return NULL;
			}
			nonce_size = CTR_NONCE_LEN;
			break;
#ifndef OPENSSL_NO_DES
		case ENCR_DES_ECB:
			key_size = 8;
			this->cipher = EVP_des_ecb();
			break;
#endif
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
	this->nonce = chunk_alloc(nonce_size);

	return &this->public;
}
