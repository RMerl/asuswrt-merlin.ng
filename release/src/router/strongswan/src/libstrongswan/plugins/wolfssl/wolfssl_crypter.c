/*
 * Copyright (C) 2019 Sean Parkinson, wolfSSL Inc.
 * Copyright (C) 2021 Andreas Steffen, strongSec GmbH
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

#include "wolfssl_crypter.h"

#include "wolfssl_common.h"
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/camellia.h>
#include <wolfssl/wolfcrypt/des3.h>

#include <utils/debug.h>

typedef struct private_wolfssl_crypter_t private_wolfssl_crypter_t;

#define CTR_SALT_LEN	4

/**
 * Private data of wolfssl_crypter_t
 */
struct private_wolfssl_crypter_t {

	/**
	 * Public part of this class.
	 */
	wolfssl_crypter_t public;

	/**
	 * wolfSSL cipher
	 */
	union {
#if !defined(NO_AES) && (!defined(NO_AES_CBC) || defined(HAVE_AES_ECB) || defined(WOLFSSL_AES_CFB) || defined(WOLFSSL_AES_COUNTER))
		Aes aes;
#endif
#ifdef HAVE_CAMELLIA
		Camellia camellia;
#endif
#ifndef NO_DES3
		Des des;
		Des3 des3;
#endif
	} cipher;

	/**
	 * Encryption algorithm identifier
	 */
	encryption_algorithm_t alg;

	/**
	 * Private key
	 */
	chunk_t key;

	/**
	 * Salt value
	 */
	chunk_t salt;

	/**
	 * Size of block
	 */
	size_t block_size;

	/**
	 * Size of IV
	 */
	size_t iv_size;
};

METHOD(crypter_t, decrypt, bool,
	private_wolfssl_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	u_char *out;
	bool success = FALSE;
	int ret;
	u_char nonce[AES_BLOCK_SIZE] = {0};
#if !defined(NO_AES) && defined(WOLFSSL_AES_COUNTER)
	chunk_t d = chunk_empty;
#endif

	out = data.ptr;
	if (dst)
	{
		*dst = chunk_alloc(data.len);
		out = dst->ptr;
	}

	if (this->salt.len > 0)
	{
		memcpy(nonce, this->salt.ptr, this->salt.len);
		memcpy(nonce + this->salt.len, iv.ptr, this->iv_size);
		nonce[AES_BLOCK_SIZE - 1] = 1;
	}

	switch (this->alg)
	{
		case ENCR_NULL:
			memcpy(out, data.ptr, data.len);
			success = TRUE;
			break;
#if !defined(NO_AES) && !defined(NO_AES_CBC)
		case ENCR_AES_CBC:
			ret = wc_AesSetKey(&this->cipher.aes, this->key.ptr, this->key.len,
							   iv.ptr, AES_DECRYPTION);
			if (ret == 0)
			{
				ret = wc_AesCbcDecrypt(&this->cipher.aes, out, data.ptr,
									   data.len);
			}
			success = (ret == 0);
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AES_ECB)
		case ENCR_AES_ECB:
			ret = wc_AesSetKey(&this->cipher.aes, this->key.ptr, this->key.len,
							   iv.ptr, AES_DECRYPTION);
			if (ret == 0)
			{
				ret = wc_AesEcbDecrypt(&this->cipher.aes, out, data.ptr,
									   data.len);
			}
			success = (ret == 0);
			break;
	#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_CFB)
		case ENCR_AES_CFB:
			ret = wc_AesSetKey(&this->cipher.aes, this->key.ptr, this->key.len,
							   iv.ptr, AES_ENCRYPTION);
			if (ret == 0)
			{
				ret = wc_AesCfbDecrypt(&this->cipher.aes, out, data.ptr,
									   data.len);
			}
			success = (ret == 0);
			break;
	#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_COUNTER)
		case ENCR_AES_CTR:
			if (out == data.ptr)
			{
				d = chunk_alloc(data.len);
				out = d.ptr;
			}
			ret = wc_AesSetKeyDirect(&this->cipher.aes, this->key.ptr,
									 this->key.len, nonce, AES_ENCRYPTION);
			if (ret == 0)
			{
				ret = wc_AesCtrEncrypt(&this->cipher.aes, out, data.ptr,
									   data.len);
			}
			if (ret == 0 && out == d.ptr)
			{
				memcpy(data.ptr, out, data.len);
			}
			chunk_free(&d);
			success = (ret == 0);
			break;
#endif
#ifdef HAVE_CAMELLIA
		case ENCR_CAMELLIA_CBC:
			ret = wc_CamelliaSetKey(&this->cipher.camellia, this->key.ptr,
									this->key.len, iv.ptr);
			if (ret == 0)
			{
				ret = wc_CamelliaCbcDecrypt(&this->cipher.camellia, out,
											data.ptr, data.len);
			}
			success = (ret == 0);
			break;
#endif
#ifndef NO_DES3
		case ENCR_3DES:
			ret = wc_Des3_SetKey(&this->cipher.des3, this->key.ptr, iv.ptr,
								 DES_DECRYPTION);
			if (ret == 0)
			{
				ret = wc_Des3_CbcDecrypt(&this->cipher.des3, out, data.ptr,
										 data.len);
			}
			success = (ret == 0);
			break;
		case ENCR_DES:
			ret = wc_Des_SetKey(&this->cipher.des, this->key.ptr, iv.ptr,
								DES_DECRYPTION);
			if (ret == 0)
			{
				ret = wc_Des_CbcDecrypt(&this->cipher.des, out, data.ptr,
										data.len);
			}
			if (ret == 0)
				success = TRUE;
			break;
	#ifdef WOLFSSL_DES_ECB
		case ENCR_DES_ECB:
			ret = wc_Des_SetKey(&this->cipher.des, this->key.ptr, iv.ptr,
								DES_DECRYPTION);
			if (ret == 0)
			{
				ret = wc_Des_EcbDecrypt(&this->cipher.des, out, data.ptr,
										data.len);
			}
			success = (ret == 0);
			break;
	#endif
#endif
		default:
			break;
	}

	memwipe(nonce, sizeof(nonce));
	return success;
}

METHOD(crypter_t, encrypt_, bool,
	private_wolfssl_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	u_char *out;
	bool success = FALSE;
	int ret;
	u_char nonce[AES_BLOCK_SIZE] = {0};
#if !defined(NO_AES) && defined(WOLFSSL_AES_COUNTER)
	chunk_t d = chunk_empty;
#endif

	out = data.ptr;
	if (dst)
	{
		*dst = chunk_alloc(data.len);
		out = dst->ptr;
	}

	if (this->salt.len > 0)
	{
		memcpy(nonce, this->salt.ptr, this->salt.len);
		memcpy(nonce + this->salt.len, iv.ptr, this->iv_size);
		nonce[AES_BLOCK_SIZE - 1] = 1;
	}

	switch (this->alg)
	{
		case ENCR_NULL:
			memcpy(out, data.ptr, data.len);
			success = TRUE;
			break;
#if !defined(NO_AES) && !defined(NO_AES_CBC)
		case ENCR_AES_CBC:
			ret = wc_AesSetKey(&this->cipher.aes, this->key.ptr, this->key.len,
							   iv.ptr, AES_ENCRYPTION);
			if (ret == 0)
			{
				ret = wc_AesCbcEncrypt(&this->cipher.aes, out, data.ptr,
									   data.len);
			}
			success = (ret == 0);
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AES_ECB)
		case ENCR_AES_ECB:
			ret = wc_AesSetKey(&this->cipher.aes, this->key.ptr, this->key.len,
							   iv.ptr, AES_ENCRYPTION);
			if (ret == 0)
			{
				ret = wc_AesEcbEncrypt(&this->cipher.aes, out, data.ptr,
									   data.len);
			}
			success = (ret == 0);
			break;
#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_CFB)
		case ENCR_AES_CFB:
			ret = wc_AesSetKey(&this->cipher.aes, this->key.ptr, this->key.len,
							   iv.ptr, AES_ENCRYPTION);
			if (ret == 0)
			{
				ret = wc_AesCfbEncrypt(&this->cipher.aes, out, data.ptr,
									   data.len);
			}
			success = (ret == 0);
			break;
#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_COUNTER)
		case ENCR_AES_CTR:
			if (out == data.ptr)
			{
				d = chunk_alloc(data.len);
				out = d.ptr;
			}
			ret = wc_AesSetKeyDirect(&this->cipher.aes, this->key.ptr,
									 this->key.len, nonce, AES_ENCRYPTION);
			if (ret == 0)
			{
				ret = wc_AesCtrEncrypt(&this->cipher.aes, out, data.ptr,
									   data.len);
			}
			if (ret == 0 && out == d.ptr)
			{
				memcpy(data.ptr, out, data.len);
			}
			chunk_free(&d);
			success = (ret == 0);
			break;
#endif
#ifdef HAVE_CAMELLIA
		case ENCR_CAMELLIA_CBC:
			ret = wc_CamelliaSetKey(&this->cipher.camellia, this->key.ptr,
									this->key.len, iv.ptr);
			if (ret == 0)
			{
				ret = wc_CamelliaCbcEncrypt(&this->cipher.camellia, out,
											data.ptr, data.len);
			}
			success = (ret == 0);
			break;
#endif
#ifndef NO_DES3
		case ENCR_3DES:
			ret = wc_Des3_SetKey(&this->cipher.des3, this->key.ptr, iv.ptr,
								 DES_ENCRYPTION);
			if (ret == 0)
			{
				ret = wc_Des3_CbcEncrypt(&this->cipher.des3, out, data.ptr,
										 data.len);
			}
			success = (ret == 0);
			break;
		case ENCR_DES:
			ret = wc_Des_SetKey(&this->cipher.des, this->key.ptr, iv.ptr,
								DES_ENCRYPTION);
			if (ret == 0)
			{
				ret = wc_Des_CbcEncrypt(&this->cipher.des, out, data.ptr,
										data.len);
			}
			success = (ret == 0);
			break;
	#ifdef WOLFSSL_DES_ECB
		case ENCR_DES_ECB:
			ret = wc_Des_SetKey(&this->cipher.des, this->key.ptr, iv.ptr,
								DES_ENCRYPTION);
			if (ret == 0)
			{
				ret = wc_Des_EcbEncrypt(&this->cipher.des, out, data.ptr,
										data.len);
			}
			success = (ret == 0);
			break;
	#endif
#endif
		default:
			break;
	}

	return success;
}

METHOD(crypter_t, get_block_size, size_t,
	private_wolfssl_crypter_t *this)
{
	return this->block_size;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_wolfssl_crypter_t *this)
{
	return this->iv_size;
}

METHOD(crypter_t, get_key_size, size_t,
	private_wolfssl_crypter_t *this)
{
	return this->key.len + this->salt.len;
}

METHOD(crypter_t, set_key, bool,
	private_wolfssl_crypter_t *this, chunk_t key)
{
	if (key.len != get_key_size(this))
	{
		return FALSE;
	}
	memcpy(this->salt.ptr, key.ptr + key.len - this->salt.len, this->salt.len);
	memcpy(this->key.ptr, key.ptr, this->key.len);
	return TRUE;
}

METHOD(crypter_t, destroy, void,
	private_wolfssl_crypter_t *this)
{
	chunk_clear(&this->key);
	chunk_clear(&this->salt);
	switch (this->alg)
	{
#if !defined(NO_AES) && !defined(NO_AES_CBC)
		case ENCR_AES_CBC:
			wc_AesFree(&this->cipher.aes);
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AES_ECB)
		case ENCR_AES_ECB:
			wc_AesFree(&this->cipher.aes);
			break;
#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_CFB)
		case ENCR_AES_CFB:
			wc_AesFree(&this->cipher.aes);
			break;
#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_COUNTER)
		case ENCR_AES_CTR:
			wc_AesFree(&this->cipher.aes);
			break;
#endif
#ifndef NO_DES3
		case ENCR_3DES:
			wc_Des3Free(&this->cipher.des3);
			break;
#endif
		default:
			break;
	}
	free(this);
}

/*
 * Described in header
 */
wolfssl_crypter_t *wolfssl_crypter_create(encryption_algorithm_t algo,
										size_t key_size)
{
	private_wolfssl_crypter_t *this;
	size_t block_size;
	size_t iv_size;
	size_t salt_len = 0;
	int ret = 0;

	switch (algo)
	{
		case ENCR_NULL:
			key_size = 0;
			block_size = 1;
			iv_size = 0;
			break;
#if !defined(NO_AES) && !defined(NO_AES_CBC)
		case ENCR_AES_CBC:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* fall-through */
				case 16:
				case 24:
				case 32:
					block_size = AES_BLOCK_SIZE;
					iv_size = AES_IV_SIZE;
					break;
				default:
					return NULL;
			}
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AES_ECB)
		case ENCR_AES_ECB:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* fall-through */
				case 16:
				case 24:
				case 32:
					block_size = AES_BLOCK_SIZE;
					iv_size = AES_IV_SIZE;
					break;
				default:
					return NULL;
			}
			break;
#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_CFB)
		case ENCR_AES_CFB:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* fall-through */
				case 16:
				case 24:
				case 32:
					block_size = AES_BLOCK_SIZE;
					iv_size = AES_IV_SIZE;
					break;
				default:
					return NULL;
			}
			break;
#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_COUNTER)
		case ENCR_AES_CTR:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* fall-through */
				case 16:
				case 24:
				case 32:
					block_size = 1;
					iv_size = 8;
					salt_len = CTR_SALT_LEN;
					break;
				default:
					return NULL;
			}
			break;
#endif
#ifdef HAVE_CAMELLIA
		case ENCR_CAMELLIA_CBC:
			switch (key_size)
			{
				case 0:
					key_size = 16;
					/* fall-through */
				case 16:
				case 24:
				case 32:
					block_size = CAMELLIA_BLOCK_SIZE;
					iv_size = CAMELLIA_BLOCK_SIZE;
					break;
				default:
					return NULL;
			}
			break;
#endif
#ifndef NO_DES3
		case ENCR_3DES:
			if (key_size != 24)
			{
				return NULL;
			}
			block_size = DES_BLOCK_SIZE;
			iv_size = DES_BLOCK_SIZE;
			break;
		case ENCR_DES:
	#ifdef WOLFSSL_DES_ECB
		case ENCR_DES_ECB:
	#endif
			if (key_size != 8)
			{
				return NULL;
			}
			block_size = DES_BLOCK_SIZE;
			iv_size = DES_BLOCK_SIZE;
			break;
#endif
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.crypter = {
				.encrypt = _encrypt_,
				.decrypt = _decrypt,
				.get_block_size = _get_block_size,
				.get_iv_size = _get_iv_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.alg = algo,
		.block_size = block_size,
		.iv_size = iv_size,
	);

	switch (algo)
	{
#if !defined(NO_AES) && !defined(NO_AES_CBC)
		case ENCR_AES_CBC:
			ret = wc_AesInit(&this->cipher.aes, NULL, INVALID_DEVID);
			break;
#endif
#if !defined(NO_AES) && defined(HAVE_AES_ECB)
		case ENCR_AES_ECB:
			ret = wc_AesInit(&this->cipher.aes, NULL, INVALID_DEVID);
			break;
#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_CFB)
		case ENCR_AES_CFB:
			ret = wc_AesInit(&this->cipher.aes, NULL, INVALID_DEVID);
			break;
#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_COUNTER)
		case ENCR_AES_CTR:
			ret = wc_AesInit(&this->cipher.aes, NULL, INVALID_DEVID);
			break;
#endif
#ifndef NO_DES3
		case ENCR_3DES:
			ret = wc_Des3Init(&this->cipher.des3, NULL, INVALID_DEVID);
			break;
#endif
		default:
			break;
	}
	if (ret != 0)
	{
		free(this);
		return NULL;
	}

	this->key = chunk_alloc(key_size);
	this->salt = chunk_alloc(salt_len);

	return &this->public;
}
