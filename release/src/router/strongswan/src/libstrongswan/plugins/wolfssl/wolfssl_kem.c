/*
 * Copyright (C) 2024-2025 Tobias Brunner, codelabs GmbH
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

#ifdef WOLFSSL_HAVE_MLKEM

#include <wolfssl/wolfcrypt/mlkem.h>
#ifdef WOLFSSL_WC_MLKEM
#include <wolfssl/wolfcrypt/wc_mlkem.h>
#endif

typedef struct private_key_exchange_t private_key_exchange_t;

/**
 * Private data.
 */
struct private_key_exchange_t {

	/**
	 * Public interface.
	 */
	key_exchange_t public;

	/**
	 * KE method.
	 */
	key_exchange_method_t method;

	/**
	 * Internal algorithm type.
	 */
	int type;

	/**
	 * Key pair as initiator.
	 */
	MlKemKey *kem;

	/**
	 * Ciphertext as responder.
	 */
	chunk_t ciphertext;

	/**
	 * Shared secret.
	 */
	chunk_t shared_secret;

#ifdef TESTABLE_KE
	/**
	 * DRBG for testing.
	 */
	drbg_t *drbg;
#endif
};

/**
 * Allocate random bytes from either the DRBG or a new WC_RNG instance to the
 * given buffer. Make sure it has enough room.
 */
static bool get_random(private_key_exchange_t *this, size_t len, uint8_t *out)
{
	WC_RNG rng;

#ifdef TESTABLE_KE
	if (this->drbg)
	{
		return this->drbg->generate(this->drbg, len, out);
	}
#endif

	if (wc_InitRng(&rng) != 0)
	{
		return FALSE;
	}
	if (wc_RNG_GenerateBlock(&rng, out, len) != 0)
	{
		wc_FreeRng(&rng);
		return FALSE;
	}
	wc_FreeRng(&rng);
	return TRUE;
}

/**
 * Generate a key pair as initiator and return the public key.
 */
static bool generate_keypair(private_key_exchange_t *this, chunk_t *public)
{
	uint8_t random[WC_ML_KEM_MAKEKEY_RAND_SZ];
	word32 len;

	if (!this->kem)
	{
		this->kem = malloc(sizeof(MlKemKey));
		if (wc_MlKemKey_Init(this->kem, this->type, NULL, INVALID_DEVID) != 0)
		{
			free(this->kem);
			this->kem = NULL;
			return FALSE;
		}
		if (!get_random(this, sizeof(random), random) ||
			wc_MlKemKey_MakeKeyWithRandom(this->kem, random, sizeof(random)) != 0)
		{
			DBG1(DBG_LIB, "%N key pair generation failed",
				 key_exchange_method_names, this->method);
			return FALSE;
		}
		memwipe(random, sizeof(random));
	}

	if (wc_MlKemKey_PublicKeySize(this->kem, &len) != 0)
	{
		return FALSE;
	}
	*public = chunk_alloc(len);

	if (wc_MlKemKey_EncodePublicKey(this->kem, public->ptr, public->len) != 0)
	{
		DBG1(DBG_LIB, "%N public key encoding failed",
				 key_exchange_method_names, this->method);
		chunk_free(public);
		return FALSE;
	}
	return TRUE;
}

METHOD(key_exchange_t, get_public_key, bool,
	private_key_exchange_t *this, chunk_t *value)
{
	/* as responder, this method is called after set_public_key(), which
	 * encapsulated the secret to produce this ciphertext */
	if (this->ciphertext.len)
	{
		*value = chunk_clone(this->ciphertext);
		return TRUE;
	}

	/* as initiator, we generate a key pair and return the public key */
	return generate_keypair(this, value);
}

/**
 * Decapsulate the shared secret from the given ciphertext using our key pair.
 */
static bool decaps_ciphertext(private_key_exchange_t *this, chunk_t ciphertext)
{
	word32 ss_len;

	if (wc_MlKemKey_SharedSecretSize(this->kem, &ss_len) != 0)
	{
		return FALSE;
	}
	this->shared_secret = chunk_alloc(ss_len);

	if (wc_MlKemKey_Decapsulate(this->kem, this->shared_secret.ptr,
								ciphertext.ptr, ciphertext.len) != 0)
	{
		DBG1(DBG_LIB, "%N decapsulation failed",
			 key_exchange_method_names, this->method);
		return FALSE;
	}
	return TRUE;
}

/**
 * Generate a shared secret an encapsulate it using the given public key.
 */
static bool encaps_shared_secret(private_key_exchange_t *this, chunk_t public)
{
	uint8_t random[WC_ML_KEM_ENC_RAND_SZ];
	MlKemKey kem;
	word32 ct_len, ss_len;

	if (wc_MlKemKey_Init(&kem, this->type, NULL, INVALID_DEVID) != 0)
	{
		return FALSE;
	}
	if (wc_MlKemKey_DecodePublicKey(&kem, public.ptr, public.len) != 0)
	{
		DBG1(DBG_LIB, "%N public key invalid",
			 key_exchange_method_names, this->method);
		wc_MlKemKey_Free(&kem);
		return FALSE;
	}

	if (wc_MlKemKey_CipherTextSize(&kem, &ct_len) != 0 ||
		wc_MlKemKey_SharedSecretSize(&kem, &ss_len) != 0)
	{
		wc_MlKemKey_Free(&kem);
		return FALSE;
	}
	this->ciphertext = chunk_alloc(ct_len);
	this->shared_secret = chunk_alloc(ss_len);

	if (!get_random(this, sizeof(random), random) ||
		wc_MlKemKey_EncapsulateWithRandom(&kem, this->ciphertext.ptr,
						this->shared_secret.ptr, random, sizeof(random)) != 0)
	{
		DBG1(DBG_LIB, "%N encapsulation failed",
			 key_exchange_method_names, this->method);
		wc_MlKemKey_Free(&kem);
		return FALSE;
	}
	memwipe(random, sizeof(random));
	wc_MlKemKey_Free(&kem);
	return TRUE;
}

METHOD(key_exchange_t, set_public_key, bool,
	private_key_exchange_t *this, chunk_t value)
{
	/* as initiator, we decapsulate the secret from the given ciphertext */
	if (this->kem)
	{
		return decaps_ciphertext(this, value);
	}

	/* as responder, we generate a secret and encapsulate it */
	return encaps_shared_secret(this, value);
}

METHOD(key_exchange_t, get_shared_secret, bool,
	private_key_exchange_t *this, chunk_t *secret)
{
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
}

METHOD(key_exchange_t, get_method, key_exchange_method_t,
	private_key_exchange_t *this)
{
	return this->method;
}

#ifdef TESTABLE_KE

METHOD(key_exchange_t, set_seed, bool,
	private_key_exchange_t *this, chunk_t value, drbg_t *drbg)
{
	if (!drbg)
	{
		return FALSE;
	}
	DESTROY_IF(this->drbg);
	this->drbg = drbg->get_ref(drbg);
	return TRUE;
}

#endif /* TESTABLE_KE */

METHOD(key_exchange_t, destroy, void,
	private_key_exchange_t *this)
{
	chunk_clear(&this->shared_secret);
	chunk_free(&this->ciphertext);
	wc_MlKemKey_Free(this->kem);
	free(this->kem);
#ifdef TESTABLE_KE
	DESTROY_IF(this->drbg);
#endif
	free(this);
}

/*
 * Described in header
 */
key_exchange_t *wolfssl_kem_create(key_exchange_method_t method)
{
	private_key_exchange_t *this;
	int type;

	switch (method)
	{
#ifdef WOLFSSL_WC_ML_KEM_512
		case ML_KEM_512:
			type = WC_ML_KEM_512;
			break;
#endif
#ifdef WOLFSSL_WC_ML_KEM_768
		case ML_KEM_768:
			type = WC_ML_KEM_768;
			break;
#endif
#ifdef WOLFSSL_WC_ML_KEM_1024
		case ML_KEM_1024:
			type = WC_ML_KEM_1024;
			break;
#endif
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.get_method = _get_method,
			.get_public_key = _get_public_key,
			.set_public_key = _set_public_key,
			.get_shared_secret = _get_shared_secret,
			.destroy = _destroy,
		},
		.method = method,
		.type = type,
	);

#ifdef TESTABLE_KE
	this->public.set_seed = _set_seed;
#endif

	return &this->public;
}

#endif /* WOLFSSL_HAVE_MLKEM */
