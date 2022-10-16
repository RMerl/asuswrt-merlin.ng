/*
 * Copyright (C) 2019 Sean Parkinson, wolfSSL Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "wolfssl_common.h"

#if (!defined(NO_AES) && (defined(HAVE_AESGCM) || defined(HAVE_AESCCM))) || \
								(defined(HAVE_CHACHA) && defined(HAVE_POLY1305))

#include "wolfssl_aead.h"

#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/chacha.h>
#include <wolfssl/wolfcrypt/chacha20_poly1305.h>
#include <crypto/iv/iv_gen_seq.h>

/** as defined in RFC 4106 */
#define IV_LEN			8
#define GCM_SALT_LEN	4
#define GCM_NONCE_LEN	(GCM_SALT_LEN + IV_LEN)

#define CCM_SALT_LEN	3
#define CCM_NONCE_LEN	(CCM_SALT_LEN + IV_LEN)

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
	chunk_t salt;

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
	union {
#if !defined(NO_AES) && (defined(HAVE_AESGCM) || defined(HAVE_AESCCM))
		Aes aes;
#endif
	} cipher;

	/**
	 * The cipher to use
	 */
	encryption_algorithm_t alg;
};

METHOD(aead_t, encrypt, bool,
	private_aead_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
	chunk_t *encrypted)
{
	chunk_t nonce;
	u_char *out;
	bool success = FALSE;
	int ret;

	out = plain.ptr;
	if (encrypted)
	{
		*encrypted = chunk_alloc(plain.len + this->icv_size);
		out = encrypted->ptr;
	}

	nonce = chunk_cata("cc", this->salt, iv);

	switch (this->alg)
	{
#if !defined(NO_AES) && defined(HAVE_AESGCM)
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
			ret = wc_AesGcmSetKey(&this->cipher.aes, this->key.ptr,
								  this->key.len);
			if (ret == 0)
			{
				ret = wc_AesGcmEncrypt(&this->cipher.aes, out, plain.ptr,
						plain.len, nonce.ptr, GCM_NONCE_LEN, out + plain.len,
						this->icv_size, assoc.ptr, assoc.len);
			}
			success = (ret == 0);
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AESCCM)
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
			/* wc_AesCcmEncrypt fails if the pointer is NULL */
			if (!plain.ptr && !plain.len)
			{
				plain.ptr = nonce.ptr;
			}
			ret = wc_AesCcmSetKey(&this->cipher.aes, this->key.ptr,
								  this->key.len);
			if (ret == 0)
			{
				ret = wc_AesCcmEncrypt(&this->cipher.aes, out, plain.ptr,
						plain.len, nonce.ptr, CCM_NONCE_LEN, out + plain.len,
						this->icv_size, assoc.ptr, assoc.len);
			}
			success = (ret == 0);
			break;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
		case ENCR_CHACHA20_POLY1305:
			ret = wc_ChaCha20Poly1305_Encrypt(this->key.ptr, nonce.ptr,
						assoc.ptr, assoc.len, plain.ptr, plain.len, out,
						out + plain.len);
			success = (ret == 0);
			break;
#endif
		default:
			break;
	}

	memwipe(nonce.ptr, nonce.len);
	return success;
}

METHOD(aead_t, decrypt, bool,
	private_aead_t *this, chunk_t encrypted, chunk_t assoc, chunk_t iv,
	chunk_t *plain)
{
	chunk_t nonce;
	u_char *out;
	bool success = FALSE;
	int ret = 0;

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

	nonce = chunk_cata("cc", this->salt, iv);

	switch (this->alg)
	{
#if !defined(NO_AES) && defined(HAVE_AESGCM)
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
			ret = wc_AesGcmSetKey(&this->cipher.aes, this->key.ptr,
								  this->key.len);
			if (ret == 0)
			{
				ret = wc_AesGcmDecrypt(&this->cipher.aes, out, encrypted.ptr,
							encrypted.len, nonce.ptr, GCM_NONCE_LEN,
							encrypted.ptr + encrypted.len, this->icv_size,
							assoc.ptr, assoc.len);
			}
			success = (ret == 0);
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AESCCM)
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
			/* wc_AesCcmDecrypt() fails if the pointers are NULL */
			if (!encrypted.ptr && !encrypted.len)
			{
				encrypted.ptr = nonce.ptr;
			}
			if (!out && !encrypted.len)
			{
				out = nonce.ptr;
			}
			ret = wc_AesCcmSetKey(&this->cipher.aes, this->key.ptr,
								  this->key.len);
			if (ret == 0)
			{
				ret = wc_AesCcmDecrypt(&this->cipher.aes, out, encrypted.ptr,
							encrypted.len, nonce.ptr, CCM_NONCE_LEN,
							encrypted.ptr + encrypted.len, this->icv_size,
							assoc.ptr, assoc.len);
			}
			success = (ret == 0);
			break;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
		case ENCR_CHACHA20_POLY1305:
			ret = wc_ChaCha20Poly1305_Decrypt(this->key.ptr, nonce.ptr,
							assoc.ptr, assoc.len, encrypted.ptr, encrypted.len,
							encrypted.ptr + encrypted.len, out);
			success = (ret == 0);
			break;
#endif
		default:
			break;
	}

	memwipe(nonce.ptr, nonce.len);
	return success;
}

METHOD(aead_t, get_block_size, size_t,
	private_aead_t *this)
{
	/* all AEAD algorithms are streaming */
	return 1;
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
	return this->key.len + this->salt.len;
}

METHOD(aead_t, set_key, bool,
	private_aead_t *this, chunk_t key)
{
	if (key.len != get_key_size(this))
	{
		return FALSE;
	}
	memcpy(this->salt.ptr, key.ptr + key.len - this->salt.len, this->salt.len);
	memcpy(this->key.ptr, key.ptr, this->key.len);
	return TRUE;
}

METHOD(aead_t, destroy, void,
	private_aead_t *this)
{
	chunk_clear(&this->key);
	chunk_clear(&this->salt);
	switch (this->alg)
	{
#if !defined(NO_AES) && defined(HAVE_AESGCM)
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
			wc_AesFree(&this->cipher.aes);
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AESCCM)
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
			wc_AesFree(&this->cipher.aes);
			break;
#endif
		default:
			break;
	}
	this->iv_gen->destroy(this->iv_gen);
	free(this);
}

/*
 * Described in header
 */
aead_t *wolfssl_aead_create(encryption_algorithm_t algo,
							size_t key_size, size_t salt_size)
{
	private_aead_t *this;
	size_t expected_salt_size;

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
		.alg = algo,
	);

	switch (algo)
	{
#if !defined(NO_AES) && defined(HAVE_AESGCM)
	#if WOLFSSL_MIN_AUTH_TAG_SZ <= 8
		case ENCR_AES_GCM_ICV8:
			this->icv_size = 8;
			break;
	#endif
	#if WOLFSSL_MIN_AUTH_TAG_SZ <= 12
		case ENCR_AES_GCM_ICV12:
			this->icv_size = 12;
			break;
	#endif
		case ENCR_AES_GCM_ICV16:
			this->icv_size = 16;
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AESCCM)
		case ENCR_AES_CCM_ICV8:
			this->icv_size = 8;
			break;
		case ENCR_AES_CCM_ICV12:
			this->icv_size = 12;
			break;
		case ENCR_AES_CCM_ICV16:
			this->icv_size = 16;
			break;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
		case ENCR_CHACHA20_POLY1305:
			this->icv_size = 16;
			break;
#endif
		default:
			free(this);
			return NULL;
	}

	switch (algo)
	{
#if !defined(NO_AES) && defined(HAVE_AESGCM)
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:
				case 24:
				case 32:
					expected_salt_size = GCM_SALT_LEN;
					if (wc_AesInit(&this->cipher.aes, NULL, INVALID_DEVID) != 0)
					{
						DBG1(DBG_LIB, "AES Init failed, aead create failed");
						free(this);
						return NULL;
					}
					break;
				default:
					free(this);
					return NULL;
			}
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AESCCM)
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* FALL */
				case 16:
				case 24:
				case 32:
					expected_salt_size = CCM_SALT_LEN;
					if (wc_AesInit(&this->cipher.aes, NULL, INVALID_DEVID) != 0)
					{
						DBG1(DBG_LIB, "AES Init failed, aead create failed");
						free(this);
						return NULL;
					}
					break;
				default:
					free(this);
					return NULL;
			}
			break;
#endif
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
		case ENCR_CHACHA20_POLY1305:
			switch (key_size)
			{
				case 0:
					key_size = 32;
					/* FALL */
				case 32:
					expected_salt_size = 4;
					break;
				default:
					free(this);
					return NULL;
			}
			break;
#endif
		default:
			free(this);
			return NULL;
	}

	if (salt_size && salt_size != expected_salt_size)
	{
		/* currently not supported */
		free(this);
		return NULL;
	}

	this->key = chunk_alloc(key_size);
	this->salt = chunk_alloc(expected_salt_size);
	this->iv_gen = iv_gen_seq_create();

	return &this->public;
}

#endif
