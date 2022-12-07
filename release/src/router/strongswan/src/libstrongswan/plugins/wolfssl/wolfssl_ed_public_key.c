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

#include "wolfssl_ed_public_key.h"

#include <utils/debug.h>
#include <asn1/asn1.h>

#ifdef HAVE_ED25519
#include <wolfssl/wolfcrypt/ed25519.h>
#endif
#ifdef HAVE_ED448
#include <wolfssl/wolfcrypt/ed448.h>
#endif

#include <wolfssl/wolfcrypt/asn.h>

typedef struct private_public_key_t private_public_key_t;

/**
 * Private data
 */
struct private_public_key_t {

	/**
	 * Public interface
	 */
	public_key_t public;

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

METHOD(public_key_t, get_type, key_type_t,
	private_public_key_t *this)
{
	return this->type;
}

METHOD(public_key_t, verify, bool,
	private_public_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t signature)
{
	byte dummy[1];
	int ret = -1, res = 0;

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
		ret = wc_ed25519_verify_msg(signature.ptr, signature.len, data.ptr,
									data.len, &res, &this->key.ed25519);
#endif
	}
	else if (this->type == KEY_ED448)
	{
#ifdef HAVE_ED448
		ret = wc_ed448_verify_msg(signature.ptr, signature.len, data.ptr,
								  data.len, &res, &this->key.ed448, NULL, 0);
#endif
	}
	return ret == 0 && res == 1;
}

METHOD(public_key_t, encrypt_, bool,
	private_public_key_t *this, encryption_scheme_t scheme,
	void *params, chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "encryption scheme %N not supported", encryption_scheme_names,
		 scheme);
	return FALSE;
}

/**
 * Returns the key size in bytes for the given type, also used in private key.
 */
int wolfssl_ed_keysize(key_type_t type)
{
	if (type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		return ED25519_KEY_SIZE * 8;
#endif
	}
	else if (type == KEY_ED448)
	{
#ifdef HAVE_ED448
		return ED448_KEY_SIZE * 8;
#endif
	}
	return 0;
}

METHOD(public_key_t, get_keysize, int,
	private_public_key_t *this)
{
	return wolfssl_ed_keysize(this->type);
}

/**
 * Encode the given public key as ASN.1 DER with algorithm identifier
 */
static bool encode_pubkey(wolfssl_ed_key *key, key_type_t type,
						  chunk_t *encoding)
{
	int ret = -1;

	/* account for algorithmIdentifier/bitString */
	if (type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		*encoding = chunk_alloc(ED25519_PUB_KEY_SIZE + 2 * MAX_SEQ_SZ +
								MAX_ALGO_SZ + TRAILING_ZERO);
		ret = wc_Ed25519PublicKeyToDer(&key->ed25519, encoding->ptr,
									   encoding->len, 1);
#endif
	}
	else if (type == KEY_ED448)
	{
#ifdef HAVE_ED448
		*encoding = chunk_alloc(ED448_PUB_KEY_SIZE + 2 * MAX_SEQ_SZ +
								MAX_ALGO_SZ + TRAILING_ZERO);
		ret = wc_Ed448PublicKeyToDer(&key->ed448, encoding->ptr,
									   encoding->len, 1);
#endif
	}
	if (ret < 0)
	{
		return FALSE;
	}
	encoding->len = ret;
	return TRUE;
}

/**
 * Export the raw public key of the given key, also used in ed private key.
 */
bool wolfssl_ed_public_key(wolfssl_ed_key *key, key_type_t type, chunk_t *raw)
{
	word32 len;

	*raw = chunk_empty;
	if (type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		len = ED25519_PUB_KEY_SIZE;
		*raw = chunk_alloc(len);
		if (wc_ed25519_export_public(&key->ed25519, raw->ptr, &len) != 0)
		{
			chunk_free(raw);
			return FALSE;
		}
#endif
	}
	else if (type == KEY_ED448)
	{
#ifdef HAVE_ED448
		len = ED448_PUB_KEY_SIZE;
		*raw = chunk_alloc(len);
		if (wc_ed448_export_public(&key->ed448, raw->ptr, &len) != 0)
		{
			chunk_free(raw);
			return FALSE;
		}
#endif
	}
	return TRUE;
}

/**
 * Calculate fingerprint from an EdDSA key, also used in ed private key.
 */
bool wolfssl_ed_fingerprint(wolfssl_ed_key *key, key_type_t key_type,
							cred_encoding_type_t type, chunk_t *fp)
{
	hasher_t *hasher;
	chunk_t blob;
	bool success = FALSE;

	if (lib->encoding->get_cache(lib->encoding, type, key, fp))
	{
		return TRUE;
	}
	switch (type)
	{
		case KEYID_PUBKEY_SHA1:
			if (!wolfssl_ed_public_key(key, key_type, &blob))
			{
				return FALSE;
			}
			break;
		case KEYID_PUBKEY_INFO_SHA1:
			if (!encode_pubkey(key, key_type, &blob))
			{
				return FALSE;
			}
			break;
		default:
			return FALSE;
	}
	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher || !hasher->allocate_hash(hasher, blob, fp))
	{
		DBG1(DBG_LIB, "SHA1 not supported, fingerprinting failed");
	}
	else
	{
		lib->encoding->cache(lib->encoding, type, key, fp);
		success = TRUE;
	}
	DESTROY_IF(hasher);
	chunk_free(&blob);
	return success;
}

METHOD(public_key_t, get_fingerprint, bool,
	private_public_key_t *this, cred_encoding_type_t type, chunk_t *fingerprint)
{
	return wolfssl_ed_fingerprint(&this->key, this->type, type, fingerprint);
}

METHOD(public_key_t, get_encoding, bool,
	private_public_key_t *this, cred_encoding_type_t type, chunk_t *encoding)
{
	bool success = TRUE;

	if (!encode_pubkey(&this->key, this->type, encoding))
	{
		return FALSE;
	}

	if (type != PUBKEY_SPKI_ASN1_DER)
	{
		chunk_t asn1_encoding = *encoding;

		success = lib->encoding->encode(lib->encoding, type,
								NULL, encoding, CRED_PART_EDDSA_PUB_ASN1_DER,
								asn1_encoding, CRED_PART_END);
		chunk_free(&asn1_encoding);
	}
	return success;
}

METHOD(public_key_t, get_ref, public_key_t*,
	private_public_key_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

/**
 * Destroy an EdDSA key of the given type, also used by ed private key.
 */
void wolfssl_ed_destroy(wolfssl_ed_key *key, key_type_t type)
{
	if (type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		wc_ed25519_free(&key->ed25519);
#endif
	}
	else if (type == KEY_ED448)
	{
#ifdef HAVE_ED448
		wc_ed448_free(&key->ed448);
#endif
	}
}

METHOD(public_key_t, destroy, void,
	private_public_key_t *this)
{
	if (ref_put(&this->ref))
	{
		lib->encoding->clear_cache(lib->encoding, &this->key);
		wolfssl_ed_destroy(&this->key, this->type);
		free(this);
	}
}

/**
 * Initialized an EdDSA key of the given type, also used by ed private key.
 */
bool wolfssl_ed_create(wolfssl_ed_key *key, key_type_t type)
{
	if (type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		if (wc_ed25519_init(&key->ed25519) != 0)
		{
			return FALSE;
		}
#endif
	}
	else if (type == KEY_ED448)
	{
#ifdef HAVE_ED448
		if (wc_ed448_init(&key->ed448) != 0)
		{
			return FALSE;
		}
#endif
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * Generic private constructor
 */
static private_public_key_t *create_empty(key_type_t type)
{
	private_public_key_t *this;

	INIT(this,
		.public = {
			.get_type = _get_type,
			.verify = _verify,
			.encrypt = _encrypt_,
			.get_keysize = _get_keysize,
			.equals = public_key_equals,
			.get_fingerprint = _get_fingerprint,
			.has_fingerprint = public_key_has_fingerprint,
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
public_key_t *wolfssl_ed_public_key_load(key_type_t type, va_list args)
{
	private_public_key_t *this;
	chunk_t blob = chunk_empty, pub = chunk_empty;
	int idx;
	int ret = -1;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_EDDSA_PUB:
				pub = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	this = create_empty(type);
	if (!this)
	{
		return NULL;
	}

	if (type == KEY_ED25519)
	{
#ifdef HAVE_ED25519
		if (pub.len)
		{
			ret = wc_ed25519_import_public(pub.ptr, pub.len,
										   &this->key.ed25519);
		}
		else if (blob.len)
		{
			idx = 0;
			ret = wc_Ed25519PublicKeyDecode(blob.ptr, &idx, &this->key.ed25519,
											blob.len);
		}
#endif
	}
	else if (type == KEY_ED448)
	{
#ifdef HAVE_ED448
		if (pub.len)
		{
			ret = wc_ed448_import_public(pub.ptr, pub.len, &this->key.ed448);
		}
		else if (blob.len)
		{
			idx = 0;
			ret = wc_Ed448PublicKeyDecode(blob.ptr, &idx, &this->key.ed448,
										  blob.len);
		}
#endif
	}

	if (ret != 0)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

#endif /* HAVE_ED25519 */
