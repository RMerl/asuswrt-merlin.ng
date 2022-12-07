/*
 * Copyright (C) 2013-2019 Tobias Brunner
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

#include <openssl/opensslv.h>

#include "openssl_aead.h"

#include <openssl/evp.h>
#include <crypto/iv/iv_gen_seq.h>

/* the generic AEAD identifiers were added with 1.1.0 */
#ifndef EVP_CTRL_AEAD_SET_IVLEN
#define EVP_CTRL_AEAD_SET_IVLEN EVP_CTRL_GCM_SET_IVLEN
#define EVP_CTRL_AEAD_SET_TAG EVP_CTRL_GCM_SET_TAG
#define EVP_CTRL_AEAD_GET_TAG EVP_CTRL_GCM_GET_TAG
#endif

/* not defined for older versions of BoringSSL */
#ifndef EVP_CIPH_CCM_MODE
#define EVP_CIPH_CCM_MODE 0xffff
#endif

/** as defined in RFC 4106 */
#define IV_LEN			8
#define SALT_LEN		4
#define NONCE_LEN		(IV_LEN + SALT_LEN)
/** as defined in RFC 4309 */
#define CCM_SALT_LEN	3

typedef struct private_aead_t private_aead_t;

/**
 * Private data of aead_t
 */
struct private_aead_t {

	/**
	 * Public interface
	 */
	aead_t public;

	/**
	 * The encryption key
	 */
	chunk_t	key;

	/**
	 * Salt value
	 */
	char salt[SALT_LEN];

	/**
	 * Size of the salt
	 */
	size_t salt_size;

	/**
	 * Size of the integrity check value
	 */
	size_t icv_size;

	/**
	 * IV generator
	 */
	iv_gen_t *iv_gen;

	/**
	 * The cipher to use
	 */
	const EVP_CIPHER *cipher;
};

/**
 * Do the actual en/decryption in an EVP context
 */
static bool crypt(private_aead_t *this, chunk_t data, chunk_t assoc, chunk_t iv,
				  u_char *out, int enc)
{
	EVP_CIPHER_CTX *ctx;
	u_char nonce[NONCE_LEN];
	bool success = FALSE;
	int len;

	memcpy(nonce, this->salt, this->salt_size);
	memcpy(nonce + this->salt_size, iv.ptr, IV_LEN);

	ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_set_padding(ctx, 0);
	if (!EVP_CipherInit_ex(ctx, this->cipher, NULL, NULL, NULL, enc) ||
		!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN,
							 this->salt_size + IV_LEN, NULL))
	{
		goto done;
	}
	if ((!enc || EVP_CIPHER_mode(this->cipher) == EVP_CIPH_CCM_MODE) &&
		!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, this->icv_size,
							 enc ? NULL : data.ptr + data.len))
	{	/* set ICV for verification on decryption, CCM requires the ICV length
		 * when encrypting */
		goto done;
	}
	if (!EVP_CipherInit_ex(ctx, NULL, NULL, this->key.ptr, nonce, enc))
	{	/* set key and nonce */
		goto done;
	}
	if (EVP_CIPHER_mode(this->cipher) == EVP_CIPH_CCM_MODE &&
		!EVP_CipherUpdate(ctx, NULL, &len, NULL, data.len))
	{	/* CCM requires setting the total input length (plain or cipher+ICV) */
		goto done;
	}
	if (assoc.len && !EVP_CipherUpdate(ctx, NULL, &len, assoc.ptr, assoc.len))
	{	/* set AAD if specified */
		goto done;
	}
	/* CCM doesn't like NULL pointers as input, make sure we don't pass one */
	if (!EVP_CipherUpdate(ctx, out, &len, data.ptr ?: out, data.len) ||
		!EVP_CipherFinal_ex(ctx, out + len, &len))
	{	/* EVP_CipherFinal_ex fails if ICV is incorrect on decryption */
		goto done;
	}
	if (enc && !EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, this->icv_size,
									out + data.len))
	{	/* copy back the ICV when encrypting */
		goto done;
	}
	success = TRUE;

done:
	EVP_CIPHER_CTX_free(ctx);
	return success;
}

METHOD(aead_t, encrypt, bool,
	private_aead_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
	chunk_t *encrypted)
{
	u_char *out;

	out = plain.ptr;
	if (encrypted)
	{
		*encrypted = chunk_alloc(plain.len + this->icv_size);
		out = encrypted->ptr;
	}
	return crypt(this, plain, assoc, iv, out, 1);
}

METHOD(aead_t, decrypt, bool,
	private_aead_t *this, chunk_t encrypted, chunk_t assoc, chunk_t iv,
	chunk_t *plain)
{
	u_char *out;

	if (encrypted.len < this->icv_size)
	{
		return FALSE;
	}
	encrypted.len -= this->icv_size;

	out = encrypted.ptr;
	if (plain)
	{
		*plain = chunk_alloc(encrypted.len);
		out = plain->ptr;
	}
	return crypt(this, encrypted, assoc, iv, out, 0);
}

METHOD(aead_t, get_block_size, size_t,
	private_aead_t *this)
{
	return EVP_CIPHER_block_size(this->cipher);
}

METHOD(aead_t, get_icv_size, size_t,
	private_aead_t *this)
{
	return this->icv_size;
}

METHOD(aead_t, get_iv_size, size_t,
	private_aead_t *this)
{
	return IV_LEN;
}

METHOD(aead_t, get_iv_gen, iv_gen_t*,
	private_aead_t *this)
{
	return this->iv_gen;
}

METHOD(aead_t, get_key_size, size_t,
	private_aead_t *this)
{
	return this->key.len + this->salt_size;
}

METHOD(aead_t, set_key, bool,
	private_aead_t *this, chunk_t key)
{
	if (key.len != get_key_size(this))
	{
		return FALSE;
	}
	memcpy(this->salt, key.ptr + key.len - this->salt_size, this->salt_size);
	memcpy(this->key.ptr, key.ptr, this->key.len);
	return TRUE;
}

METHOD(aead_t, destroy, void,
	private_aead_t *this)
{
	chunk_clear(&this->key);
	this->iv_gen->destroy(this->iv_gen);
	free(this);
}

/*
 * Described in header
 */
aead_t *openssl_aead_create(encryption_algorithm_t algo,
							size_t key_size, size_t salt_size)
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
		.salt_size = SALT_LEN,
	);

	switch (algo)
	{
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_CCM_ICV8:
			this->icv_size = 8;
			break;
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_CCM_ICV12:
			this->icv_size = 12;
			break;
		case ENCR_AES_GCM_ICV16:
		case ENCR_AES_CCM_ICV16:
			this->icv_size = 16;
			break;
		case ENCR_CHACHA20_POLY1305:
			this->icv_size = 16;
			break;
		default:
			free(this);
			return NULL;
	}

	switch (algo)
	{
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:
					this->cipher = EVP_aes_128_gcm();
					break;
				case 24:
					this->cipher = EVP_aes_192_gcm();
					break;
				case 32:
					this->cipher = EVP_aes_256_gcm();
					break;
				default:
					free(this);
					return NULL;
			}
			break;
#if OPENSSL_VERSION_NUMBER >= 0x1010000fL
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:
					this->cipher = EVP_aes_128_ccm();
					break;
				case 24:
					this->cipher = EVP_aes_192_ccm();
					break;
				case 32:
					this->cipher = EVP_aes_256_ccm();
					break;
				default:
					free(this);
					return NULL;
			}
			this->salt_size = CCM_SALT_LEN;
			break;
#endif /* OPENSSL_VERSION_NUMBER */
#if OPENSSL_VERSION_NUMBER >= 0x1010000fL && !defined(OPENSSL_NO_CHACHA)
		case ENCR_CHACHA20_POLY1305:
			switch (key_size)
			{
				case 0:
					key_size = 32;
					/* FALL */
				case 32:
					this->cipher = EVP_chacha20_poly1305();
					break;
				default:
					free(this);
					return NULL;
			}
			break;
#endif /* OPENSSL_NO_CHACHA */
		default:
			free(this);
			return NULL;
	}

	if (salt_size && salt_size != this->salt_size)
	{
		/* currently not supported */
		free(this);
		return NULL;
	}

	if (!this->cipher)
	{
		free(this);
		return NULL;
	}

	this->key = chunk_alloc(key_size);
	this->iv_gen = iv_gen_seq_create();

	return &this->public;
}
