/*
 * Copyright (C) 2020 Tobias Brunner
 *
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

#if defined(HAVE_ED25519) || defined(HAVE_ED448)

#include "wolfssl_ed_private_key.h"

#include <utils/debug.h>

#include <wolfssl/wolfcrypt/asn.h>

typedef struct private_private_key_t private_private_key_t;

/**
 * Private data
 */
struct private_private_key_t {

	/**
	 * Public interface
	 */
	private_key_t public;

	/**
	 * Key object
	 */
	wolfssl_ed_key key;

	/**
	 * Key type
	 */
	key_type_t type;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

/* from ed public key */
int wolfssl_ed_keysize(key_type_t type);
bool wolfssl_ed_create(wolfssl_ed_key *key, key_type_t type);
void wolfssl_ed_destroy(wolfssl_ed_key *key, key_type_t type);
bool wolfssl_ed_public_key(wolfssl_ed_key *key, key_type_t type, chunk_t *raw);
bool wolfssl_ed_fingerprint(wolfssl_ed_key *key, key_type_t key_type,
							cred_encoding_type_t type, chunk_t *fp);

METHOD(private_key_t, sign, bool,
	private_private_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t *signature)
{
	word32 len;
	byte dummy[1];
	int ret = -1;

	if ((this->type == KEY_ED25519 && scheme != SIGN_ED25519) ||
		(this->type == KEY_ED448 && scheme != SIGN_ED448))
	{
		DBG1(DBG_LIB, "signature scheme %N not supported by %N key",
			 signature_scheme_names, scheme, key_type_names, this->type);
		return FALSE;
	}

	if (!data.ptr && !data.len)
	{
		data.ptr = dummy;
	}

	if (this->type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		len = ED25519_SIG_SIZE;
		*signature = chunk_alloc(len);
		ret = wc_ed25519_sign_msg(data.ptr, data.len, signature->ptr, &len,
								  &this->key.ed25519);
#endif
	}
	else if (this->type == KEY_ED448)
	{
#ifdef HAVE_ED448
		len = ED448_SIG_SIZE;
		*signature = chunk_alloc(len);
		ret = wc_ed448_sign_msg(data.ptr, data.len, signature->ptr, &len,
								&this->key.ed448, NULL, 0);
#endif
	}
	return ret == 0;
}

METHOD(private_key_t, decrypt, bool,
	private_private_key_t *this, encryption_scheme_t scheme,
	void *params, chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "EdDSA private key decryption not implemented");
	return FALSE;
}

METHOD(private_key_t, get_keysize, int,
	private_private_key_t *this)
{
	return wolfssl_ed_keysize(this->type);
}

METHOD(private_key_t, get_type, key_type_t,
	private_private_key_t *this)
{
	return this->type;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_private_key_t *this)
{
	public_key_t *public;
	chunk_t key;

	if (!wolfssl_ed_public_key(&this->key, this->type, &key))
	{
		return NULL;
	}
	public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, this->type,
								BUILD_EDDSA_PUB, key, BUILD_END);
	chunk_free(&key);
	return public;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return wolfssl_ed_fingerprint(&this->key, this->type, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_private_key_t *this, cred_encoding_type_t type, chunk_t *encoding)
{
	int ret = -1;

	switch (type)
	{
		case PRIVKEY_ASN1_DER:
		case PRIVKEY_PEM:
		{
			bool success = TRUE;

			/* +4 is for the two octet strings */
			*encoding = chunk_empty;
			if (this->type == KEY_ED25519)
			{
#ifdef HAVE_ED25519
				*encoding = chunk_alloc(ED25519_PRV_KEY_SIZE + 2 * MAX_SEQ_SZ +
										MAX_VERSION_SZ + MAX_ALGO_SZ + 4);
				ret = wc_Ed25519PrivateKeyToDer(&this->key.ed25519,
												encoding->ptr, encoding->len);
#endif
			}
			else if (this->type == KEY_ED448)
			{
#ifdef HAVE_ED448
				*encoding = chunk_alloc(ED448_PRV_KEY_SIZE + 2 * MAX_SEQ_SZ +
										MAX_VERSION_SZ + MAX_ALGO_SZ + 4);
				ret = wc_Ed448PrivateKeyToDer(&this->key.ed448, encoding->ptr,
											  encoding->len);
#endif
			}
			if (ret < 0)
			{
				chunk_free(encoding);
				return FALSE;
			}
			encoding->len = ret;

			if (type == PRIVKEY_PEM)
			{
				chunk_t asn1_encoding = *encoding;

				success = lib->encoding->encode(lib->encoding, PRIVKEY_PEM,
								NULL, encoding, CRED_PART_EDDSA_PRIV_ASN1_DER,
								asn1_encoding, CRED_PART_END);
				chunk_clear(&asn1_encoding);
			}
			return success;
		}
		default:
			return FALSE;
	}
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(private_key_t, destroy, void,
	private_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		lib->encoding->clear_cache(lib->encoding, &this->key);
		wolfssl_ed_destroy(&this->key, this->type);
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_private_key_t *create_internal(key_type_t type)
{
	private_private_key_t *this;

	INIT(this,
		.public = {
			.get_type = _get_type,
			.sign = _sign,
			.decrypt = _decrypt,
			.get_keysize = _get_keysize,
			.get_public_key = _get_public_key,
			.equals = private_key_equals,
			.belongs_to = private_key_belongs_to,
			.get_fingerprint = _get_fingerprint,
			.has_fingerprint = private_key_has_fingerprint,
			.get_encoding = _get_encoding,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.type = type,
		.ref = 1,
	);

	if (!wolfssl_ed_create(&this->key, type))
	{
		free(this);
		this = NULL;
	}
	return this;
}

/*
 * Described in header
 */
private_key_t *wolfssl_ed_private_key_gen(key_type_t type, va_list args)
{
	private_private_key_t *this;
	WC_RNG rng;
	int ret = -1;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_KEY_SIZE:
				/* just ignore the key size */
				va_arg(args, u_int);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	this = create_internal(type);
	if (!this)
	{
		return NULL;
	}

	if (wc_InitRng(&rng) != 0)
	{
		DBG1(DBG_LIB, "initializing random failed");
		destroy(this);
		return NULL;
	}

	if (type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		ret = wc_ed25519_make_key(&rng, ED25519_KEY_SIZE, &this->key.ed25519);
#endif
	}
	else if (type == KEY_ED448)
	{
#ifdef HAVE_ED448
		ret = wc_ed448_make_key(&rng, ED448_KEY_SIZE, &this->key.ed448);
#endif
	}
	wc_FreeRng(&rng);

	if (ret < 0)
	{
		DBG1(DBG_LIB, "generating %N key failed", key_type_names, type);
		destroy(this);
		return NULL;
	}
	return &this->public;
}

/**
 * Fix the internal state if only the private key is set
 */
static int set_public_key(private_private_key_t *this)
{
	int ret = 0;

	if (this->type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		if (!this->key.ed25519.pubKeySet)
		{
			ret = wc_ed25519_make_public(&this->key.ed25519,
									this->key.ed25519.p, ED25519_PUB_KEY_SIZE);
			if (ret == 0)
			{
				/* put public key after private key in the same buffer */
				memmove(this->key.ed25519.k + ED25519_KEY_SIZE,
						this->key.ed25519.p, ED25519_PUB_KEY_SIZE);
				this->key.ed25519.pubKeySet = 1;
			}
		}
#endif
	}
	else if (this->type == KEY_ED448)
	{
#ifdef HAVE_ED448
		if (!this->key.ed448.pubKeySet)
		{
			ret = wc_ed448_make_public(&this->key.ed448, this->key.ed448.p,
									   ED448_PUB_KEY_SIZE);
			if (ret == 0)
			{
				/* put public key after private key in the same buffer */
				memmove(this->key.ed448.k + ED448_KEY_SIZE,
						this->key.ed448.p, ED448_PUB_KEY_SIZE);
				this->key.ed448.pubKeySet = 1;
			}
		}
#endif
	}
	return ret;
}

/*
 * Described in header
 */
private_key_t *wolfssl_ed_private_key_load(key_type_t type, va_list args)
{
	private_private_key_t *this;
	chunk_t blob = chunk_empty, priv = chunk_empty;
	int idx;
	int ret = -1;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_EDDSA_PRIV_ASN1_DER:
				priv = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	this = create_internal(type);
	if (!this)
	{
		return NULL;
	}

	if (type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		if (priv.len)
		{	/* check for ASN.1 wrapped key (Octet String == 0x04) */
			if (priv.len == ED25519_KEY_SIZE + 2 &&
				priv.ptr[0] == 0x04 && priv.ptr[1] == ED25519_KEY_SIZE)
			{
				priv = chunk_skip(priv, 2);
			}
			ret = wc_ed25519_import_private_only(priv.ptr, priv.len,
												 &this->key.ed25519);
		}
		else if (blob.len)
		{
			idx = 0;
			ret = wc_Ed25519PrivateKeyDecode(blob.ptr, &idx, &this->key.ed25519,
											 blob.len);
		}
#endif
	}
	else if (type == KEY_ED448)
	{
#ifdef HAVE_ED448
		if (priv.len)
		{	/* check for ASN.1 wrapped key (Octet String == 0x04) */
			if (priv.len == ED448_KEY_SIZE + 2 &&
				priv.ptr[0] == 0x04 && priv.ptr[1] == ED448_KEY_SIZE)
			{
				priv = chunk_skip(priv, 2);
			}
			ret = wc_ed448_import_private_only(priv.ptr, priv.len,
											   &this->key.ed448);
		}
		else if (blob.len)
		{
			idx = 0;
			ret = wc_Ed448PrivateKeyDecode(blob.ptr, &idx, &this->key.ed448,
										   blob.len);
		}
#endif
	}

	if (ret == 0)
	{
		ret = set_public_key(this);
	}
	if (ret != 0)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

#endif /* HAVE_ED25519 */
