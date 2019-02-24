/*
 * Copyright (C) 2018 René Korthaus
 * Copyright (C) 2018 Konstantinos Kolelis
 * Copyright (C) 2018 Tobias Hommel
 * Rohde & Schwarz Cybersecurity GmbH
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

#include "botan_crypter.h"

#include <botan/build.h>

#if defined(BOTAN_HAS_AES) && defined(BOTAN_HAS_MODE_CBC)

#include <botan/ffi.h>

typedef struct private_botan_crypter_t private_botan_crypter_t;

/**
 * Private data of botan_crypter_t
 */
struct private_botan_crypter_t {

	/**
	 * Public part of this class
	 */
	botan_crypter_t public;

	/**
	 * The key
	 */
	chunk_t	key;

	/**
	 * The cipher name
	 */
	const char* cipher_name;
};

/**
 * Do the actual en/decryption
 */
static bool crypt(private_botan_crypter_t *this, chunk_t data, chunk_t iv,
				  chunk_t *dst, uint32_t init_flag)
{
	botan_cipher_t cipher;
	size_t output_written = 0;
	size_t input_consumed = 0;
	uint8_t *in, *out;
	bool success = FALSE;

	in = data.ptr;
	if (dst)
	{
		*dst = chunk_alloc(data.len);
		out = dst->ptr;
	}
	else
	{
		out = data.ptr;
	}

	if (botan_cipher_init(&cipher, this->cipher_name, init_flag))
	{
		return FALSE;
	}

	if (!botan_cipher_set_key(cipher, this->key.ptr, this->key.len) &&
		!botan_cipher_start(cipher, iv.ptr, iv.len) &&
		!botan_cipher_update(cipher, BOTAN_CIPHER_UPDATE_FLAG_FINAL, out,
							 data.len, &output_written, in, data.len,
							 &input_consumed) &&
		(output_written == input_consumed))
	{
		success = TRUE;
	}

	botan_cipher_destroy(cipher);
	return success;
}

METHOD(crypter_t, decrypt, bool,
	private_botan_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	return crypt(this, data, iv, dst, BOTAN_CIPHER_INIT_FLAG_DECRYPT);
}


METHOD(crypter_t, encrypt, bool,
	private_botan_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	return crypt(this, data, iv, dst, BOTAN_CIPHER_INIT_FLAG_ENCRYPT);
}

METHOD(crypter_t, get_block_size, size_t,
	private_botan_crypter_t *this)
{
	return AES_BLOCK_SIZE;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_botan_crypter_t *this)
{
	return AES_BLOCK_SIZE;
}

METHOD(crypter_t, get_key_size, size_t,
	private_botan_crypter_t *this)
{
	return this->key.len;
}

METHOD(crypter_t, set_key, bool,
	private_botan_crypter_t *this, chunk_t key)
{
	memcpy(this->key.ptr, key.ptr, min(key.len, this->key.len));
	return TRUE;
}

METHOD(crypter_t, destroy, void,
	private_botan_crypter_t *this)
{
	chunk_clear(&this->key);
	free(this);
}

/*
 * Described in header
 */
botan_crypter_t *botan_crypter_create(encryption_algorithm_t algo,
									  size_t key_size)
{
	private_botan_crypter_t *this;

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
		case ENCR_AES_CBC:
			switch (key_size)
			{
				case 16:
					/* AES 128 */
					this->cipher_name = "AES-128/CBC/NoPadding";
					break;
				case 24:
					/* AES-192 */
					this->cipher_name = "AES-192/CBC/NoPadding";
					break;
				case 32:
					/* AES-256 */
					this->cipher_name = "AES-256/CBC/NoPadding";
					break;
				default:
					free(this);
					return NULL;
			}
			break;
		default:
			free(this);
			return NULL;
	}

	this->key = chunk_alloc(key_size);
	return &this->public;
}

#endif
