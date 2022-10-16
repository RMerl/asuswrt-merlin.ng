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

#ifndef NO_DH

#include <wolfssl/wolfcrypt/dh.h>

#include "wolfssl_diffie_hellman.h"
#include "wolfssl_util.h"

#include <utils/debug.h>

typedef struct private_wolfssl_diffie_hellman_t private_wolfssl_diffie_hellman_t;

/**
 * Private data of an wolfssl_diffie_hellman_t object.
 */
struct private_wolfssl_diffie_hellman_t {

	/**
	 * Public wolfssl_diffie_hellman_t interface.
	 */
	wolfssl_diffie_hellman_t public;

	/**
	 * Diffie Hellman group number.
	 */
	diffie_hellman_group_t group;

	/**
	 * Diffie Hellman object
	 */
	DhKey dh;

	/**
	 * Length of public values
	 */
	int len;

	/**
	 * Private key
	 */
	chunk_t priv;

	/**
	 * Public key
	 */
	chunk_t pub;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;
};

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_wolfssl_diffie_hellman_t *this, chunk_t *value)
{
	*value = chunk_copy_pad(chunk_alloc(this->len), this->pub, 0x00);
	return TRUE;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_wolfssl_diffie_hellman_t *this, chunk_t *secret)
{
	if (!this->shared_secret.len)
	{
		return FALSE;
	}
	*secret = chunk_copy_pad(chunk_alloc(this->len), this->shared_secret, 0x00);
	return TRUE;
}

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_wolfssl_diffie_hellman_t *this, chunk_t value)
{
	word32 len;

	if (!diffie_hellman_verify_value(this->group, value))
	{
		return FALSE;
	}

	chunk_clear(&this->shared_secret);
	this->shared_secret = chunk_alloc(this->len);
	if (wc_DhAgree(&this->dh, this->shared_secret.ptr, &len, this->priv.ptr,
				   this->priv.len, value.ptr, value.len) != 0)
	{
		DBG1(DBG_LIB, "DH shared secret computation failed");
		chunk_free(&this->shared_secret);
		return FALSE;
	}
	this->shared_secret.len = len;
	return TRUE;
}

METHOD(diffie_hellman_t, set_private_value, bool,
	private_wolfssl_diffie_hellman_t *this, chunk_t value)
{
	bool success = FALSE;
	chunk_t g;
	word32 len;

	chunk_clear(&this->priv);
	this->priv = chunk_clone(value);

	/* calculate public value - g^priv mod p */
	if (wolfssl_mp2chunk(&this->dh.g, &g))
	{
		len = this->pub.len;
		if (wc_DhAgree(&this->dh, this->pub.ptr, &len, this->priv.ptr,
						 this->priv.len, g.ptr, g.len) == 0)
		{
			this->pub.len = len;
			success = TRUE;
		}
	}

	free(g.ptr);
	return success;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_wolfssl_diffie_hellman_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, destroy, void,
	private_wolfssl_diffie_hellman_t *this)
{
	wc_FreeDhKey(&this->dh);
	chunk_clear(&this->pub);
	chunk_clear(&this->priv);
	chunk_clear(&this->shared_secret);
	free(this);
}

/**
 * Maximum private key length when generating key
 */
static int wolfssl_priv_key_size(int len)
{
	if (len <= 128)
	{
		return 21;
	}
	if (len <= 256)
	{
		return 29;
	}
	if (len <= 384)
	{
		return 34;
	}
	if (len <= 512)
	{
		return 39;
	}
	if (len <= 640)
	{
		return 42;
	}
	if (len <= 768)
	{
		return 46;
	}
	if (len <= 896)
	{
		return 49;
	}
	if (len <= 1024)
	{
		return 52;
	}
	return len / 20;
}

/**
 * Generic internal constructor
 */
static wolfssl_diffie_hellman_t *create_generic(diffie_hellman_group_t group,
												chunk_t g, chunk_t p)
{
	private_wolfssl_diffie_hellman_t *this;
	word32 privLen, pubLen;
	WC_RNG rng;

	INIT(this,
		.public = {
			.dh = {
				.get_shared_secret = _get_shared_secret,
				.set_other_public_value = _set_other_public_value,
				.get_my_public_value = _get_my_public_value,
				.set_private_value = _set_private_value,
				.get_dh_group = _get_dh_group,
				.destroy = _destroy,
			},
		},
		.group = group,
		.len = p.len,
	);

	if (wc_InitDhKey(&this->dh) != 0)
	{
		free(this);
		return NULL;
	}

	if (wc_DhSetKey(&this->dh, p.ptr, p.len, g.ptr, g.len) != 0)
	{
		destroy(this);
		return NULL;
	}

	if (wc_InitRng(&rng) != 0)
	{
		destroy(this);
		return NULL;
	}

	this->priv = chunk_alloc(wolfssl_priv_key_size(this->len));
	this->pub = chunk_alloc(this->len);
	privLen = this->priv.len;
	pubLen = this->pub.len;
	/* generate my public and private values */
	if (wc_DhGenerateKeyPair(&this->dh, &rng, this->priv.ptr, &privLen,
							 this->pub.ptr, &pubLen) != 0)
	{
		wc_FreeRng(&rng);
		destroy(this);
		return NULL;
	}
	this->pub.len = pubLen;
	this->priv.len = privLen;
	wc_FreeRng(&rng);

	return &this->public;
}

/*
 * Described in header
 */
wolfssl_diffie_hellman_t *wolfssl_diffie_hellman_create(
											diffie_hellman_group_t group, ...)
{
	diffie_hellman_params_t *params;
	chunk_t g, p;

	if (group == MODP_CUSTOM)
	{
		VA_ARGS_GET(group, g, p);
		return create_generic(group, g, p);
	}
	params = diffie_hellman_get_params(group);
	if (!params)
	{
		return NULL;
	}
	/* wolfSSL doesn't support optimized exponent sizes according to RFC 3526 */
	return create_generic(group, params->generator, params->prime);
}

#endif /* NO_DH */
