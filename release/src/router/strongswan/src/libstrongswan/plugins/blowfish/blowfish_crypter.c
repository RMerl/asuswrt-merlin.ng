/*
 * Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are adhered to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the routines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publicly available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include "blowfish.h"

/* Blowfish as implemented from 'Blowfish: Springer-Verlag paper'
 * (From LECTURE NOTES IN COMPUTER SCIENCE 809, FAST SOFTWARE ENCRYPTION,
 * CAMBRIDGE SECURITY WORKSHOP, CAMBRIDGE, U.K., DECEMBER 9-11, 1993)
 */

#include "blowfish_crypter.h"

typedef struct private_blowfish_crypter_t private_blowfish_crypter_t;

/**
 * Class implementing the Blowfish symmetric encryption algorithm.
 */
struct private_blowfish_crypter_t {

	/**
	 * Public part of this class.
	 */
	blowfish_crypter_t public;

	/**
	 * Blowfish key schedule
	 */
	BF_KEY schedule;

	/**
	* Key size of this Blowfish cipher object.
	*/
	uint32_t key_size;
};

METHOD(crypter_t, decrypt, bool,
	private_blowfish_crypter_t *this, chunk_t data, chunk_t iv,
	chunk_t *decrypted)
{
	uint8_t *in, *out;

	if (decrypted)
	{
		*decrypted = chunk_alloc(data.len);
		out = decrypted->ptr;
	}
	else
	{
		out = data.ptr;
	}
	in = data.ptr;
	iv = chunk_clone(iv);

	BF_cbc_encrypt(in, out, data.len, &this->schedule, iv.ptr, 0);

	free(iv.ptr);

	return TRUE;
}

METHOD(crypter_t, encrypt, bool,
	private_blowfish_crypter_t *this, chunk_t data, chunk_t iv,
	chunk_t *encrypted)
{
	uint8_t *in, *out;

	if (encrypted)
	{
		*encrypted = chunk_alloc(data.len);
		out = encrypted->ptr;
	}
	else
	{
		out = data.ptr;
	}
	in = data.ptr;
	iv = chunk_clone(iv);

	BF_cbc_encrypt(in, out, data.len, &this->schedule, iv.ptr, 1);

	free(iv.ptr);

	return TRUE;
}

METHOD(crypter_t, get_block_size, size_t,
	private_blowfish_crypter_t *this)
{
	return BLOWFISH_BLOCK_SIZE;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_blowfish_crypter_t *this)
{
	return BLOWFISH_BLOCK_SIZE;
}

METHOD(crypter_t, get_key_size, size_t,
	private_blowfish_crypter_t *this)
{
	return this->key_size;
}

METHOD(crypter_t, set_key, bool,
	private_blowfish_crypter_t *this, chunk_t key)
{
	BF_set_key(&this->schedule, key.len , key.ptr);
	return TRUE;
}

METHOD(crypter_t, destroy, void,
	private_blowfish_crypter_t *this)
{
	memwipe(this, sizeof(*this));
	free(this);
}

/*
 * Described in header
 */
blowfish_crypter_t *blowfish_crypter_create(encryption_algorithm_t algo,
											size_t key_size)
{
	private_blowfish_crypter_t *this;

	if (algo != ENCR_BLOWFISH)
	{
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
		.key_size = key_size ?: 16,
	);

	return &this->public;
}
