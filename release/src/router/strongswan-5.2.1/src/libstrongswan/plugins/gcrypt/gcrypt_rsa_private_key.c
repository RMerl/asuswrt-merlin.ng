/*
 * Copyright (C) 2005-2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <gcrypt.h>

#include "gcrypt_rsa_private_key.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>

typedef struct private_gcrypt_rsa_private_key_t private_gcrypt_rsa_private_key_t;

/**
 * Private data of a gcrypt_rsa_private_key_t object.
 */
struct private_gcrypt_rsa_private_key_t {

	/**
	 * Public interface
	 */
	gcrypt_rsa_private_key_t public;

	/**
	 * gcrypt S-expression representing an RSA key
	 */
	gcry_sexp_t key;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * find a token in a S-expression. If a key is given, its length is used to
 * pad the output to a given length.
 */
chunk_t gcrypt_rsa_find_token(gcry_sexp_t sexp, char *name, gcry_sexp_t key)
{
	gcry_sexp_t token;
	chunk_t data = chunk_empty, tmp;
	size_t len = 0;

	token = gcry_sexp_find_token(sexp, name, 1);
	if (token)
	{
		data.ptr = (char*)gcry_sexp_nth_data(token, 1, &data.len);
		if (!data.ptr)
		{
			data.len = 0;
		}
		else
		{
			if (key)
			{
				/* gcrypt might return more bytes than necessary. Truncate
				 * to key length if key given, or prepend zeros if needed  */
				len = gcry_pk_get_nbits(key);
				len = len / 8 + (len % 8 ? 1 : 0);
				if (len > data.len)
				{
					tmp = chunk_alloc(len);
					len -= data.len;
					memset(tmp.ptr, 0, tmp.len - len);
					memcpy(tmp.ptr + len, data.ptr, data.len);
					data = tmp;
				}
				else if (len < data.len)
				{
					data = chunk_clone(chunk_skip(data, data.len - len));
				}
				else
				{
					data = chunk_clone(data);
				}
			}
			else
			{
				data = chunk_clone(data);
			}
		}
		gcry_sexp_release(token);
	}
	return data;
}

/**
 * Sign a chunk of data with direct PKCS#1 encoding, no hash OID
 */
static bool sign_raw(private_gcrypt_rsa_private_key_t *this,
					 chunk_t data, chunk_t *signature)
{
	gcry_sexp_t in, out;
	gcry_error_t err;
	chunk_t em;
	size_t k;

	/* EM = 0x00 || 0x01 || PS || 0x00 || T
	 * PS = 0xFF padding, with length to fill em
	 * T  = data
	 */
	k = gcry_pk_get_nbits(this->key) / 8;
	if (data.len > k - 3)
	{
		return FALSE;
	}
	em = chunk_alloc(k);
	memset(em.ptr, 0xFF, em.len);
	em.ptr[0] = 0x00;
	em.ptr[1] = 0x01;
	em.ptr[em.len - data.len - 1] = 0x00;
	memcpy(em.ptr + em.len - data.len, data.ptr, data.len);

	err = gcry_sexp_build(&in, NULL, "(data(flags raw)(value %b))",
						  em.len, em.ptr);
	chunk_free(&em);
	if (err)
	{
		DBG1(DBG_LIB, "building signature S-expression failed: %s",
			 gpg_strerror(err));
		return FALSE;
	}
	err = gcry_pk_sign(&out, in, this->key);
	gcry_sexp_release(in);
	if (err)
	{
		DBG1(DBG_LIB, "creating pkcs1 signature failed: %s", gpg_strerror(err));
		return FALSE;
	}
	*signature = gcrypt_rsa_find_token(out, "s", this->key);
	gcry_sexp_release(out);
	return !!signature->len;
}

/**
 * Sign a chunk of data using hashing and PKCS#1 encoding
 */
static bool sign_pkcs1(private_gcrypt_rsa_private_key_t *this,
					   hash_algorithm_t hash_algorithm, char *hash_name,
					   chunk_t data, chunk_t *signature)
{
	hasher_t *hasher;
	chunk_t hash;
	gcry_error_t err;
	gcry_sexp_t in, out;
	int hash_oid;

	hash_oid = hasher_algorithm_to_oid(hash_algorithm);
	if (hash_oid == OID_UNKNOWN)
	{
		return FALSE;
	}
	hasher = lib->crypto->create_hasher(lib->crypto, hash_algorithm);
	if (!hasher || !hasher->allocate_hash(hasher, data, &hash))
	{
		DESTROY_IF(hasher);
		return FALSE;
	}
	hasher->destroy(hasher);

	err = gcry_sexp_build(&in, NULL, "(data(flags pkcs1)(hash %s %b))",
						  hash_name, hash.len, hash.ptr);
	chunk_free(&hash);
	if (err)
	{
		DBG1(DBG_LIB, "building signature S-expression failed: %s", gpg_strerror(err));
		return FALSE;
	}
	err = gcry_pk_sign(&out, in, this->key);
	gcry_sexp_release(in);
	if (err)
	{
		DBG1(DBG_LIB, "creating pkcs1 signature failed: %s", gpg_strerror(err));
		return FALSE;
	}
	*signature = gcrypt_rsa_find_token(out, "s", this->key);
	gcry_sexp_release(out);
	return !!signature->len;
}

METHOD(private_key_t, get_type, key_type_t,
	private_gcrypt_rsa_private_key_t *this)
{
	return KEY_RSA;
}

METHOD(private_key_t, sign, bool,
	private_gcrypt_rsa_private_key_t *this, signature_scheme_t scheme,
	chunk_t data, chunk_t *sig)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return sign_raw(this, data, sig);
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return sign_pkcs1(this, HASH_SHA1, "sha1", data, sig);
		case SIGN_RSA_EMSA_PKCS1_SHA224:
			return sign_pkcs1(this, HASH_SHA224, "sha224", data, sig);
		case SIGN_RSA_EMSA_PKCS1_SHA256:
			return sign_pkcs1(this, HASH_SHA256, "sha256", data, sig);
		case SIGN_RSA_EMSA_PKCS1_SHA384:
			return sign_pkcs1(this, HASH_SHA384, "sha384", data, sig);
		case SIGN_RSA_EMSA_PKCS1_SHA512:
			return sign_pkcs1(this, HASH_SHA512, "sha512", data, sig);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return sign_pkcs1(this, HASH_MD5, "md5", data, sig);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_gcrypt_rsa_private_key_t *this, encryption_scheme_t scheme,
	chunk_t encrypted, chunk_t *plain)
{
	gcry_error_t err;
	gcry_sexp_t in, out;
	chunk_t padded;
	u_char *pos = NULL;;

	if (scheme != ENCRYPT_RSA_PKCS1)
	{
		DBG1(DBG_LIB, "encryption scheme %N not supported",
			 encryption_scheme_names, scheme);
		return FALSE;
	}
	err = gcry_sexp_build(&in, NULL, "(enc-val(flags)(rsa(a %b)))",
						  encrypted.len, encrypted.ptr);
	if (err)
	{
		DBG1(DBG_LIB, "building decryption S-expression failed: %s",
			 gpg_strerror(err));
		return FALSE;
	}
	err = gcry_pk_decrypt(&out, in, this->key);
	gcry_sexp_release(in);
	if (err)
	{
		DBG1(DBG_LIB, "decrypting pkcs1 data failed: %s", gpg_strerror(err));
		return FALSE;
	}
	padded.ptr = (u_char*)gcry_sexp_nth_data(out, 1, &padded.len);
	/* result is padded, but gcrypt strips leading zero:
	 *  00 | 02 | RANDOM | 00 | DATA */
	if (padded.ptr && padded.len > 2 && padded.ptr[0] == 0x02)
	{
		pos = memchr(padded.ptr, 0x00, padded.len - 1);
		if (pos)
		{
			pos++;
			*plain = chunk_clone(chunk_create(
										pos, padded.len - (pos - padded.ptr)));
		}
	}
	gcry_sexp_release(out);
	if (!pos)
	{
		DBG1(DBG_LIB, "decrypted data has invalid pkcs1 padding");
		return FALSE;
	}
	return TRUE;
}

METHOD(private_key_t, get_keysize, int,
	private_gcrypt_rsa_private_key_t *this)
{
	return gcry_pk_get_nbits(this->key);
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_gcrypt_rsa_private_key_t *this)
{
	chunk_t n, e;
	public_key_t *public;

	n = gcrypt_rsa_find_token(this->key, "n", NULL);
	e = gcrypt_rsa_find_token(this->key, "e", NULL);

	public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
						BUILD_RSA_MODULUS, n, BUILD_RSA_PUB_EXP, e, BUILD_END);
	chunk_free(&n);
	chunk_free(&e);

	return public;
}

METHOD(private_key_t, get_encoding, bool,
	private_gcrypt_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	chunk_t cn, ce, cp, cq, cd, cu, cexp1 = chunk_empty, cexp2 = chunk_empty;
	gcry_mpi_t p = NULL, q = NULL, d = NULL, exp1, exp2;
	gcry_error_t err;
	bool success;

	/* p and q are swapped, gcrypt expects p < q */
	cp = gcrypt_rsa_find_token(this->key, "q", NULL);
	cq = gcrypt_rsa_find_token(this->key, "p", NULL);
	cd = gcrypt_rsa_find_token(this->key, "d", NULL);

	err = gcry_mpi_scan(&p, GCRYMPI_FMT_USG, cp.ptr, cp.len, NULL)
		| gcry_mpi_scan(&q, GCRYMPI_FMT_USG, cq.ptr, cq.len, NULL)
		| gcry_mpi_scan(&d, GCRYMPI_FMT_USG, cd.ptr, cd.len, NULL);
	if (err)
	{
		gcry_mpi_release(p);
		gcry_mpi_release(q);
		gcry_mpi_release(d);
		chunk_clear(&cp);
		chunk_clear(&cq);
		chunk_clear(&cd);
		DBG1(DBG_LIB, "scanning mpi for export failed: %s", gpg_strerror(err));
		return FALSE;
	}

	gcry_mpi_sub_ui(p, p, 1);
	exp1 = gcry_mpi_new(gcry_pk_get_nbits(this->key));
	gcry_mpi_mod(exp1, d, p);
	gcry_mpi_release(p);

	gcry_mpi_sub_ui(q, q, 1);
	exp2 = gcry_mpi_new(gcry_pk_get_nbits(this->key));
	gcry_mpi_mod(exp2, d, q);
	gcry_mpi_release(q);

	err = gcry_mpi_aprint(GCRYMPI_FMT_USG, &cexp1.ptr, &cexp1.len, exp1)
		| gcry_mpi_aprint(GCRYMPI_FMT_USG, &cexp2.ptr, &cexp2.len, exp2);

	gcry_mpi_release(d);
	gcry_mpi_release(exp1);
	gcry_mpi_release(exp2);

	if (err)
	{
		DBG1(DBG_LIB, "printing mpi for export failed: %s", gpg_strerror(err));
		chunk_clear(&cp);
		chunk_clear(&cq);
		chunk_clear(&cd);
		chunk_clear(&cexp1);
		chunk_clear(&cexp2);
		return FALSE;
	}

	cn = gcrypt_rsa_find_token(this->key, "n", NULL);
	ce = gcrypt_rsa_find_token(this->key, "e", NULL);
	cu = gcrypt_rsa_find_token(this->key, "u", NULL);

	success = lib->encoding->encode(lib->encoding, type, NULL, encoding,
						CRED_PART_RSA_MODULUS, cn,
						CRED_PART_RSA_PUB_EXP, ce, CRED_PART_RSA_PRIV_EXP, cd,
						CRED_PART_RSA_PRIME1, cp, CRED_PART_RSA_PRIME2, cq,
						CRED_PART_RSA_EXP1, cexp1, CRED_PART_RSA_EXP2, cexp2,
						CRED_PART_RSA_COEFF, cu, CRED_PART_END);
	chunk_free(&cn);
	chunk_free(&ce);
	chunk_clear(&cd);
	chunk_clear(&cp);
	chunk_clear(&cq);
	chunk_clear(&cexp1);
	chunk_clear(&cexp2);
	chunk_clear(&cu);

	return success;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_gcrypt_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fp)
{
	chunk_t n, e;
	bool success;

	if (lib->encoding->get_cache(lib->encoding, type, this, fp))
	{
		return TRUE;
	}
	n = gcrypt_rsa_find_token(this->key, "n", NULL);
	e = gcrypt_rsa_find_token(this->key, "e", NULL);

	success = lib->encoding->encode(lib->encoding,
								type, this, fp, CRED_PART_RSA_MODULUS, n,
								CRED_PART_RSA_PUB_EXP, e, CRED_PART_END);
	chunk_free(&n);
	chunk_free(&e);
	return success;
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_gcrypt_rsa_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_gcrypt_rsa_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		gcry_sexp_release(this->key);
		lib->encoding->clear_cache(lib->encoding, this);
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_gcrypt_rsa_private_key_t *create_empty()
{
	private_gcrypt_rsa_private_key_t *this;

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

	return this;
}

/**
 * See header.
 */
gcrypt_rsa_private_key_t *gcrypt_rsa_private_key_gen(key_type_t type,
													 va_list args)
{
	private_gcrypt_rsa_private_key_t *this;
	gcry_sexp_t param;
	gcry_error_t err;
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

	err = gcry_sexp_build(&param, NULL, "(genkey(rsa(nbits %d)))", key_size);
	if (err)
	{
		DBG1(DBG_LIB, "building S-expression failed: %s", gpg_strerror(err));
		return NULL;
	}
	this = create_empty();
	err = gcry_pk_genkey(&this->key, param);
	gcry_sexp_release(param);
	if (err)
	{
		free(this);
		DBG1(DBG_LIB, "generating RSA key failed: %s", gpg_strerror(err));
		return NULL;
	}
	return &this->public;
}

/**
 * See header.
 */
gcrypt_rsa_private_key_t *gcrypt_rsa_private_key_load(key_type_t type,
													  va_list args)
{
	private_gcrypt_rsa_private_key_t *this;
	chunk_t n, e, d, p, q, u;
	gcry_error_t err;

	n = e = d = p = q = u = chunk_empty;
	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
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
				/* swap p and q, gcrypt expects p < q */
				q = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIME2:
				p = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_EXP1:
			case BUILD_RSA_EXP2:
				/* not required for gcrypt */
				va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_COEFF:
				u = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	this = create_empty();
	err = gcry_sexp_build(&this->key, NULL,
					"(private-key(rsa(n %b)(e %b)(d %b)(p %b)(q %b)(u %b)))",
					n.len, n.ptr, e.len, e.ptr, d.len, d.ptr,
					p.len, p.ptr, q.len, q.ptr, u.len, u.ptr);
	if (err)
	{
		DBG1(DBG_LIB, "loading private key failed: %s", gpg_strerror(err));
		free(this);
		return NULL;
	}
	err = gcry_pk_testkey(this->key);
	if (err)
	{
		DBG1(DBG_LIB, "private key sanity check failed: %s", gpg_strerror(err));
		destroy(this);
		return NULL;
	}
	return &this->public;
}

