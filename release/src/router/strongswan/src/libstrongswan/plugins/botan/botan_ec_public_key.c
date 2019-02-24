/*
 * Copyright (C) 2018 René Korthaus
 * Copyright (C) 2018 Konstantinos Kolelis
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

#include "botan_ec_public_key.h"
#include "botan_util.h"

#include <botan/build.h>

#ifdef BOTAN_HAS_ECDSA

#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>

#include <utils/debug.h>

#include <botan/ffi.h>

typedef struct private_botan_ec_public_key_t private_botan_ec_public_key_t;

/**
 * Private data structure with signing context.
 */
struct private_botan_ec_public_key_t {

	/**
	 * Public interface for this signer
	 */
	botan_ec_public_key_t public;

	/**
	 * Botan ec public key
	 */
	botan_pubkey_t key;

	/**
	 * Reference counter
	 */
	refcount_t ref;
};

#define SIG_FORMAT_IEEE_1363 0
#define SIG_FORMAT_DER_SEQUENCE 1

/**
 * Verification of a DER encoded signature as in RFC 3279 or as in RFC 4754
 */
static bool verify_signature(private_botan_ec_public_key_t *this,
	const char* hash_and_padding, int signature_format, size_t keylen,
	chunk_t data, chunk_t signature)
{
	chunk_t sig = signature;

	if (signature_format == SIG_FORMAT_DER_SEQUENCE)
	{
		/*
		 * botan requires a signature in IEEE 1363 format (r||s)
		 * re-encode from ASN.1 sequence of two integers r,s
		 */
		chunk_t parse = signature, r, s;

		if (asn1_unwrap(&parse, &parse) != ASN1_SEQUENCE ||
			asn1_unwrap(&parse, &r) != ASN1_INTEGER ||
			asn1_unwrap(&parse, &s) != ASN1_INTEGER)
		{
			return FALSE;
		}

		r = chunk_skip_zero(r);
		s = chunk_skip_zero(s);

		/*
		 * r and s must be of size m_order.bytes()/2 each
		 */
		if (r.len > keylen || s.len > keylen)
		{
			return FALSE;
		}

		sig = chunk_alloca(2 * keylen);
		memset(sig.ptr, 0, sig.len);
		memcpy(sig.ptr + (keylen - r.len), r.ptr, r.len);
		memcpy(sig.ptr + keylen + (keylen - s.len), s.ptr, s.len);
	}
	return botan_verify_signature(this->key, hash_and_padding, data, sig);
}

METHOD(public_key_t, get_type, key_type_t,
	private_botan_ec_public_key_t *this)
{
	return KEY_ECDSA;
}

METHOD(public_key_t, get_keysize, int,
	private_botan_ec_public_key_t *this)
{
	botan_mp_t p;
	size_t bits = 0;

	if (botan_mp_init(&p))
	{
		return 0;
	}

	if (botan_pubkey_get_field(p, this->key, "p") ||
		botan_mp_num_bits(p, &bits))
	{
		botan_mp_destroy(p);
		return 0;
	}

	botan_mp_destroy(p);
	return bits;
}

METHOD(public_key_t, verify, bool,
	private_botan_ec_public_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t signature)
{
	size_t keylen = (get_keysize(this) + 7) / 8;
	const char *hash_and_padding;
	int sig_format;

	switch (scheme)
	{
		/* r||s -> Botan::IEEE_1363, data is the hash already */
		case SIGN_ECDSA_WITH_NULL:
			hash_and_padding = "Raw";
			sig_format = SIG_FORMAT_IEEE_1363;
			break;
		/* DER SEQUENCE of two INTEGERS r,s -> Botan::DER_SEQUENCE */
		case SIGN_ECDSA_WITH_SHA1_DER:
			hash_and_padding = "EMSA1(SHA-1)";
			sig_format = SIG_FORMAT_DER_SEQUENCE;
			break;
		case SIGN_ECDSA_WITH_SHA256_DER:
			hash_and_padding = "EMSA1(SHA-256)";
			sig_format = SIG_FORMAT_DER_SEQUENCE;
			break;
		case SIGN_ECDSA_WITH_SHA384_DER:
			hash_and_padding = "EMSA1(SHA-384)";
			sig_format = SIG_FORMAT_DER_SEQUENCE;
			break;
		case SIGN_ECDSA_WITH_SHA512_DER:
			hash_and_padding = "EMSA1(SHA-512)";
			sig_format = SIG_FORMAT_DER_SEQUENCE;
			break;
		/* r||s -> Botan::IEEE_1363 */
		case SIGN_ECDSA_256:
			hash_and_padding = "EMSA1(SHA-256)";
			sig_format = SIG_FORMAT_IEEE_1363;
			break;
		case SIGN_ECDSA_384:
			hash_and_padding = "EMSA1(SHA-384)";
			sig_format = SIG_FORMAT_IEEE_1363;
			break;
		case SIGN_ECDSA_521:
			hash_and_padding = "EMSA1(SHA-512)";
			sig_format = SIG_FORMAT_IEEE_1363;
			break;
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported via botan",
				 signature_scheme_names, scheme);
			return FALSE;
	}

	return verify_signature(this, hash_and_padding,
							sig_format, keylen, data, signature);
}

METHOD(public_key_t, encrypt, bool,
	private_botan_ec_public_key_t *this, encryption_scheme_t scheme,
	chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "EC public key encryption not implemented");
	return FALSE;
}

METHOD(public_key_t, get_fingerprint, bool,
	private_botan_ec_public_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return botan_get_fingerprint(this->key, this, type, fingerprint);
}

METHOD(public_key_t, get_encoding, bool,
	private_botan_ec_public_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	return botan_get_encoding(this->key, type, encoding);
}

METHOD(public_key_t, get_ref, public_key_t*,
	private_botan_ec_public_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(public_key_t, destroy, void,
	private_botan_ec_public_key_t *this)
{
	if (ref_put(&this->ref))
	{
		lib->encoding->clear_cache(lib->encoding, this);
		botan_pubkey_destroy(this->key);
		free(this);
	}
}

/*
 * Described in header
 */
botan_ec_public_key_t *botan_ec_public_key_adopt(botan_pubkey_t key)
{
	private_botan_ec_public_key_t *this;

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.verify = _verify,
				.encrypt = _encrypt,
				.get_keysize = _get_keysize,
				.equals = public_key_equals,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = public_key_has_fingerprint,
				.get_encoding = _get_encoding,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.key = key,
		.ref = 1,
	);

	return &this->public;
}

#endif
