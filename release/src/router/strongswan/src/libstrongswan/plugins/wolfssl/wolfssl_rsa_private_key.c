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

#ifndef NO_RSA

#include "wolfssl_rsa_private_key.h"
#include "wolfssl_rsa_public_key.h"
#include "wolfssl_util.h"

#include <utils/debug.h>
#include <crypto/hashers/hasher.h>
#include <credentials/keys/signature_params.h>

#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/asn.h>

typedef struct private_wolfssl_rsa_private_key_t private_wolfssl_rsa_private_key_t;

/**
 * Private data of a wolfssl_rsa_private_key_t object
 */
struct private_wolfssl_rsa_private_key_t {

	/**
	 * Public interface
	 */
	wolfssl_rsa_private_key_t public;

	/**
	 * RSA key object from wolfSSL
	 */
	RsaKey rsa;

	/**
	 * Random number generator to use with RSA operations.
	 */
	WC_RNG rng;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

/* implemented in rsa public key */
bool wolfssl_rsa_encode_public(RsaKey *rsa, chunk_t *encoding);
bool wolfssl_rsa_fingerprint(RsaKey *rsa, cred_encoding_type_t type, chunk_t *fp);

/**
 * Build RSA signature
 */
static bool build_signature(private_wolfssl_rsa_private_key_t *this,
							enum wc_HashType hash, chunk_t data, chunk_t *sig)
{
	int ret = wc_RsaSSL_Sign(data.ptr, data.len, sig->ptr, sig->len, &this->rsa,
							 &this->rng);
	if (ret > 0)
	{
		sig->len = ret;
	}
	return ret > 0;
}

/**
 * Build an EMSA PKCS1 signature described in PKCS#1
 */
static bool build_emsa_pkcs1_signature(private_wolfssl_rsa_private_key_t *this,
									   enum wc_HashType hash, chunk_t data,
									   chunk_t *sig)
{
	bool success = FALSE;
	chunk_t dgst, digestInfo;
	int len;

	*sig = chunk_alloc(wc_RsaEncryptSize(&this->rsa));

	if (hash == WC_HASH_TYPE_NONE)
	{
		success = build_signature(this, hash, data, sig);
	}
	else if (wolfssl_hash_chunk(hash, data, &dgst))
	{
		digestInfo = chunk_alloc(MAX_DER_DIGEST_SZ);
		len = wc_EncodeSignature(digestInfo.ptr, dgst.ptr, dgst.len,
								 wc_HashGetOID(hash));
		if (len > 0)
		{
			digestInfo.len = len;
			success = build_signature(this, hash, digestInfo, sig);
		}
		chunk_free(&digestInfo);
		chunk_free(&dgst);
	}

	if (!success)
	{
		chunk_free(sig);
	}
	return success;
}

#ifdef WC_RSA_PSS
/**
 * Build an EMSA PSS signature described in PKCS#1
 */
static bool build_emsa_pss_signature(private_wolfssl_rsa_private_key_t *this,
									 rsa_pss_params_t *params, chunk_t data,
									 chunk_t *sig)
{
	bool success = FALSE;
	chunk_t dgst = chunk_empty;
	enum wc_HashType hash;
	int mgf, ret;

	if (!wolfssl_hash2type(params->hash, &hash))
	{
		return FALSE;
	}
	if (!wolfssl_hash2mgf1(params->mgf1_hash, &mgf))
	{
		return FALSE;
	}

	*sig = chunk_alloc(wc_RsaEncryptSize(&this->rsa));

	if (wolfssl_hash_chunk(hash, data, &dgst))
	{
		ret = wc_RsaPSS_Sign_ex(dgst.ptr, dgst.len, sig->ptr, sig->len, hash,
								mgf, params->salt_len, &this->rsa, &this->rng);
		if (ret > 0)
		{
			sig->len = ret;
			success = TRUE;
		}
	}

	chunk_free(&dgst);
	if (!success)
	{
		chunk_free(sig);
	}
	return success;
}
#endif


METHOD(private_key_t, get_type, key_type_t,
	private_wolfssl_rsa_private_key_t *this)
{
	return KEY_RSA;
}

METHOD(private_key_t, sign, bool,
	private_wolfssl_rsa_private_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_NONE, data,
											  signature);
#ifdef WOLFSSL_SHA224
		case SIGN_RSA_EMSA_PKCS1_SHA2_224:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_SHA224, data,
											  signature);
#endif
#ifndef NO_SHA256
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_SHA256, data,
											  signature);
#endif
#ifdef WOLFSSL_SHA384
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_SHA384, data,
											  signature);
#endif
#ifdef WOLFSSL_SHA512
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_SHA512, data,
											  signature);
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_224)
		case SIGN_RSA_EMSA_PKCS1_SHA3_224:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_SHA3_224,
											  data, signature);
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
		case SIGN_RSA_EMSA_PKCS1_SHA3_256:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_SHA3_256,
											  data, signature);
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_384)
		case SIGN_RSA_EMSA_PKCS1_SHA3_384:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_SHA3_384,
											  data, signature);
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
		case SIGN_RSA_EMSA_PKCS1_SHA3_512:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_SHA3_512,
											 data, signature);
#endif
#ifndef NO_SHA
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_SHA, data,
											  signature);
#endif
#ifndef NO_MD5
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return build_emsa_pkcs1_signature(this, WC_HASH_TYPE_MD5, data,
											  signature);
#endif
#ifdef WC_RSA_PSS
		case SIGN_RSA_EMSA_PSS:
			return build_emsa_pss_signature(this, params, data, signature);
#endif
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported via wolfssl",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_wolfssl_rsa_private_key_t *this, encryption_scheme_t scheme,
	void *params, chunk_t crypto, chunk_t *plain)
{
	int padding, mgf, len;
	enum wc_HashType hash;
	chunk_t label = chunk_empty;

	if (params)
	{
		label = *(chunk_t *)params;
	}

	switch (scheme)
	{
		case ENCRYPT_RSA_PKCS1:
			hash = WC_HASH_TYPE_NONE;
			padding = WC_RSA_PKCSV15_PAD;
			mgf = WC_MGF1NONE;
			break;
#ifndef WC_NO_RSA_OAEP
	#ifndef NO_SHA
		case ENCRYPT_RSA_OAEP_SHA1:
			hash = WC_HASH_TYPE_SHA;
			padding = WC_RSA_OAEP_PAD;
			mgf = WC_MGF1SHA1;
			break;
	#endif
	#ifdef WOLFSSL_SHA224
		case ENCRYPT_RSA_OAEP_SHA224:
			hash = WC_HASH_TYPE_SHA224;
			padding = WC_RSA_OAEP_PAD;
			mgf = WC_MGF1SHA224;
			break;
	#endif
	#ifndef NO_SHA256
		case ENCRYPT_RSA_OAEP_SHA256:
			hash = WC_HASH_TYPE_SHA256;
			padding = WC_RSA_OAEP_PAD;
			mgf = WC_MGF1SHA256;
			break;
	#endif
	#ifdef WOLFSSL_SHA384
		case ENCRYPT_RSA_OAEP_SHA384:
			hash = WC_HASH_TYPE_SHA384;
			padding = WC_RSA_OAEP_PAD;
			mgf = WC_MGF1SHA384;
			break;
	#endif
	#ifdef WOLFSSL_SHA512
		case ENCRYPT_RSA_OAEP_SHA512:
			hash = WC_HASH_TYPE_SHA512;
			padding = WC_RSA_OAEP_PAD;
			mgf = WC_MGF1SHA512;
			break;
	#endif
#endif
		default:
			DBG1(DBG_LIB, "encryption scheme %N not supported via wolfssl",
				 encryption_scheme_names, scheme);
			return FALSE;
	}
	len = wc_RsaEncryptSize(&this->rsa);
	*plain = chunk_alloc(len);
	len = wc_RsaPrivateDecrypt_ex(crypto.ptr, crypto.len, plain->ptr, len,
								  &this->rsa, padding, hash, mgf,
								  label.ptr, label.len);
	if (len < 0)
	{
		DBG1(DBG_LIB, "RSA decryption failed");
		chunk_free(plain);
		return FALSE;
	}
	plain->len = len;
	return TRUE;
}

METHOD(private_key_t, get_keysize, int,
	private_wolfssl_rsa_private_key_t *this)
{
	return wc_RsaEncryptSize(&this->rsa) * 8;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_wolfssl_rsa_private_key_t *this)
{
	public_key_t *key;
	chunk_t enc;

	if (!wolfssl_rsa_encode_public(&this->rsa, &enc))
	{
		return NULL;
	}
	key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
							 BUILD_BLOB_ASN1_DER, enc, BUILD_END);
	chunk_free(&enc);
	return key;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_wolfssl_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return wolfssl_rsa_fingerprint(&this->rsa, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_wolfssl_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	switch (type)
	{
		case PRIVKEY_ASN1_DER:
		case PRIVKEY_PEM:
		{
			bool success = TRUE;
			int len;

			/* n and d are of keysize length, p and q plus the three CRT
			 * params roughly half that, the version and e are small */
			len = wc_RsaEncryptSize(&this->rsa) * 5 + MAX_SEQ_SZ;
			*encoding = chunk_alloc(len);
			len = wc_RsaKeyToDer(&this->rsa, encoding->ptr, len);
			if (len < 0)
			{
				chunk_free(encoding);
				return FALSE;
			}
			encoding->len = len;

			if (type == PRIVKEY_PEM)
			{
				chunk_t asn1_encoding = *encoding;

				success = lib->encoding->encode(lib->encoding, PRIVKEY_PEM,
								NULL, encoding, CRED_PART_RSA_PRIV_ASN1_DER,
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
	private_wolfssl_rsa_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_wolfssl_rsa_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		lib->encoding->clear_cache(lib->encoding, &this->rsa);
		wc_FreeRsaKey(&this->rsa);
		wc_FreeRng(&this->rng);
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_wolfssl_rsa_private_key_t *create_empty()
{
	private_wolfssl_rsa_private_key_t *this;

	INIT(this,
		.public = {
			.key = {
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
		},
		.ref = 1,
	);

	if (wc_InitRng(&this->rng) != 0)
	{
		DBG1(DBG_LIB, "init RNG failed, rsa private key create failed");
		free(this);
		return NULL;
	}
	if (wc_InitRsaKey(&this->rsa, NULL) != 0)
	{
		DBG1(DBG_LIB, "init RSA failed, rsa private key create failed");
		wc_FreeRng(&this->rng);
		free(this);
		return NULL;
	}
#ifdef WC_RSA_BLINDING
	this->rsa.rng = &this->rng;
#endif

	return this;
}

/*
 * Described in header
 */
wolfssl_rsa_private_key_t *wolfssl_rsa_private_key_gen(key_type_t type,
													   va_list args)
{
	private_wolfssl_rsa_private_key_t *this;
	u_int key_size = 0;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_KEY_SIZE:
				key_size = va_arg(args, u_int);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (!key_size)
	{
		return NULL;
	}

	this = create_empty();
	if (!this)
	{
		return NULL;
	}

	if (wc_MakeRsaKey(&this->rsa, key_size, WC_RSA_EXPONENT, &this->rng) < 0)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

/**
 * Allocate a random number in the range [0, n-1]
 */
static bool wolfssl_mp_rand(mp_int *n, WC_RNG *rng, mp_int *r)
{
	int len, ret;

	/* ensure the number has enough memory. */
	ret = mp_set_bit(r, mp_count_bits(n));
	if (ret == 0)
	{
		len = sizeof(*r->dp) * n->used;
		ret = wc_RNG_GenerateBlock(rng, (byte *)r->dp, len);
	}
	if (ret == 0)
	{
		ret = mp_mod(r, n, r);
	}
	return ret == 0;
}

/**
 * Recover the primes from n, e and d using the algorithm described in
 * Appendix C of NIST SP 800-56B.
 */
static bool calculate_pq(mp_int *n, mp_int *e, mp_int *d, mp_int *p, mp_int *q,
						 mp_int *t1, mp_int *t2, WC_RNG* rng)
{
	int i, t, j;
	bool success = FALSE;
	mp_int *k = p;
	mp_int *r = p;
	mp_int *n1 = q;
	mp_int *g = t2;
	mp_int *y = t2;
	mp_int *x = t1;

	/* k = (d * e) - 1 */
	if (mp_mul(d, e, k) != 0)
	{
		goto error;
	}
	if (mp_sub_d(k, 1, k) != 0)
	{
		goto error;
	}
	/* k must be even */
	if (mp_isodd(k))
	{
		goto error;
	}
	/* k = 2^t * r, where r is the largest odd integer dividing k, and t >= 1 */
	if (mp_copy(k, r) != 0)
	{
		goto error;
	}
	for (t = 0; !mp_isodd(r); t++)
	{   /* r = r/2 */
		if (mp_div_2(r, r) != 0)
			goto error;
	}
	/* we need n-1 below */
	if (mp_sub_d(n, 1, n1) != 0)
	{
		goto error;
	}
	for (i = 0; i < 100; i++)
	{   /* generate random integer g in [0, n-1] */
		if (!wolfssl_mp_rand(n, rng, g))
		{
			goto error;
		}
		/* y = g^r mod n */
		if (mp_exptmod(g, r, n, y) != 0)
		{
			goto error;
		}
		/* try again if y == 1 or y == n-1 */
		if (mp_isone(y) || mp_cmp(y, n1) == MP_EQ)
		{
			continue;
		}
		for (j = 0; j < t; j++)
		{   /* x = y^2 mod n */
			if (mp_sqrmod(y, n, x) != 0)
			{
				goto error;
			}
			/* stop if x == 1 */
			if (mp_isone(x))
			{
				goto done;
			}
			/* retry with new g if x = n-1 */
			if (mp_cmp(x, n1) == MP_EQ)
			{
				break;
			}
			/* y = x */
			if (mp_copy(x, y) != 0)
			{
				goto error;
			}
		}
	}
	goto error;

done:
	/* p = gcd(y-1, n) */
	if (mp_sub_d(y, 1, y) != 0)
	{
		goto error;
	}
	if (mp_gcd(y, n, p) != 0)
	{
		goto error;
	}
	/* q = n/p */
	if (mp_div(n, p, q, NULL) != 0)
	{
		goto error;
	}

	success = TRUE;

error:
	return success;
}

/**
 * Calculates dp = d (mod p-1) or dq = d (mod q-1) for the Chinese remainder
 * algorithm.
 */
static bool dmodpq1(mp_int *d, mp_int *pq, mp_int *res)
{
	/* p|q - 1
	 * d (mod p|q -1) */
	return mp_sub_d(pq, 1, res) == 0 &&
		   mp_mod(d, res, res) == 0;
}

/**
 * Calculates qinv = q^-1 (mod p) for the Chinese remainder algorithm.
 */
static int qinv(mp_int *q, mp_int *p, mp_int *res)
{
	/* q^-1 (mod p) */
	return mp_invmod(q, p, res) == 0;
}

/*
 * Described in header
 */
wolfssl_rsa_private_key_t *wolfssl_rsa_private_key_load(key_type_t type,
														va_list args)
{
	private_wolfssl_rsa_private_key_t *this;
	chunk_t blob, n, e, d, p, q, exp1, exp2, coeff;
	word32 idx;
	int ret;

	blob = n = e = d = p = q = exp1 = exp2 = coeff = chunk_empty;
	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_MODULUS:
				n = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PUB_EXP:
				e = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIV_EXP:
				d = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIME1:
				p = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIME2:
				q = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_EXP1:
				exp1 = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_EXP2:
				exp2 = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_COEFF:
				coeff = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	this = create_empty();
	if (!this)
	{
		return NULL;
	}

	if (blob.ptr)
	{
		idx = 0;
		ret = wc_RsaPrivateKeyDecode(blob.ptr, &idx, &this->rsa, blob.len);
		if (ret == 0)
		{
			return &this->public;
		}
	}
	else if (n.ptr && e.ptr && d.ptr)
	{
		this->rsa.type = RSA_PRIVATE;

		if (mp_read_unsigned_bin(&this->rsa.n, n.ptr, n.len) != 0)
		{
			goto error;
		}
		if (mp_read_unsigned_bin(&this->rsa.e, e.ptr, e.len) != 0)
		{
			goto error;
		}
		if (mp_read_unsigned_bin(&this->rsa.d, d.ptr, d.len) != 0)
		{
			goto error;
		}
		if (p.ptr && q.ptr)
		{
			if (mp_read_unsigned_bin(&this->rsa.p, p.ptr, p.len) != 0)
			{
				goto error;
			}
			if (mp_read_unsigned_bin(&this->rsa.q, q.ptr, q.len) != 0)
			{
				goto error;
			}
		}
		else if (!calculate_pq(&this->rsa.n, &this->rsa.e, &this->rsa.d,
							   &this->rsa.p, &this->rsa.q, &this->rsa.dP,
							   &this->rsa.dQ, &this->rng))
		{
			goto error;
		}
		if (exp1.ptr)
		{
			if (mp_read_unsigned_bin(&this->rsa.dP, exp1.ptr, exp1.len) != 0)
			{
				goto error;
			}
		}
		else if (!dmodpq1(&this->rsa.d, &this->rsa.p, &this->rsa.dP))
		{
			goto error;
		}
		if (exp2.ptr)
		{
			if (mp_read_unsigned_bin(&this->rsa.dQ, exp2.ptr, exp2.len) != 0)
			{
				goto error;
			}
		}
		else if (!dmodpq1(&this->rsa.d, &this->rsa.q, &this->rsa.dQ))
		{
			goto error;
		}
		if (coeff.ptr)
		{
			if (mp_read_unsigned_bin(&this->rsa.u, coeff.ptr, coeff.len) != 0)
			{
				goto error;
			}
		}
		else if (!qinv(&this->rsa.q, &this->rsa.p, &this->rsa.u))
		{
			goto error;
		}

		return &this->public;
	}
error:
	destroy(this);
	return NULL;
}

#endif /* NO_RSA */
