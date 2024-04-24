/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include "openssl_util.h"

#include <utils/debug.h>

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/x509.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
/* for EVP_PKEY_CTX_set_dh_pad */
#include <openssl/dh.h>
#endif

/* these were added with 1.1.0 when ASN1_OBJECT was made opaque */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define OBJ_get0_data(o) ((o)->data)
#define OBJ_length(o) ((o)->length)
#define ASN1_STRING_get0_data(a) ASN1_STRING_data((ASN1_STRING*)a)
#endif

/*
 * Described in header
 */
bool openssl_compute_shared_key(EVP_PKEY *priv, EVP_PKEY *pub, chunk_t *shared)
{
	EVP_PKEY_CTX *ctx;
	bool success = FALSE;

	ctx = EVP_PKEY_CTX_new(priv, NULL);
	if (!ctx)
	{
		return FALSE;
	}

	if (EVP_PKEY_derive_init(ctx) <= 0)
	{
		goto error;
	}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (EVP_PKEY_base_id(priv) == EVP_PKEY_DH &&
		EVP_PKEY_CTX_set_dh_pad(ctx, 1) <= 0)
	{
		goto error;
	}
#endif

	if (EVP_PKEY_derive_set_peer(ctx, pub) <= 0)
	{
		goto error;
	}

	if (EVP_PKEY_derive(ctx, NULL, &shared->len) <= 0)
	{
		goto error;
	}

	*shared = chunk_alloc(shared->len);

	if (EVP_PKEY_derive(ctx, shared->ptr, &shared->len) <= 0)
	{
		chunk_clear(shared);
		goto error;
	}

	success = TRUE;

error:
	EVP_PKEY_CTX_free(ctx);
	return success;
}

/*
 * Described in header
 */
bool openssl_fingerprint(EVP_PKEY *key, cred_encoding_type_t type, chunk_t *fp)
{
	hasher_t *hasher;
	chunk_t enc;
	u_char *p;

	if (lib->encoding->get_cache(lib->encoding, type, key, fp))
	{
		return TRUE;
	}
	switch (type)
	{
		case KEYID_PUBKEY_SHA1:
			enc = chunk_alloc(i2d_PublicKey(key, NULL));
			p = enc.ptr;
			i2d_PublicKey(key, &p);
			break;
		case KEYID_PUBKEY_INFO_SHA1:
			enc = chunk_alloc(i2d_PUBKEY(key, NULL));
			p = enc.ptr;
			i2d_PUBKEY(key, &p);
			break;
		default:
			return FALSE;
	}
	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher || !hasher->allocate_hash(hasher, enc, fp))
	{
		DBG1(DBG_LIB, "SHA1 not supported, fingerprinting failed");
		DESTROY_IF(hasher);
		free(enc.ptr);
		return FALSE;
	}
	free(enc.ptr);
	hasher->destroy(hasher);
	lib->encoding->cache(lib->encoding, type, key, fp);
	return TRUE;
}

/**
 * Described in header.
 */
bool openssl_bn_cat(const int len, const BIGNUM *a, const BIGNUM *b,
					chunk_t *chunk)
{
	int offset;

	chunk->len = len + (b ? len : 0);
	chunk->ptr = malloc(chunk->len);
	memset(chunk->ptr, 0, chunk->len);

	/* convert a */
	offset = len - BN_num_bytes(a);
	if (!BN_bn2bin(a, chunk->ptr + offset))
	{
		goto error;
	}

	/* optionally convert and concatenate b */
	if (b)
	{
		offset = len - BN_num_bytes(b);
		if (!BN_bn2bin(b, chunk->ptr + len + offset))
		{
			goto error;
		}
	}

	return TRUE;
error:
	chunk_free(chunk);
	return FALSE;
}

/**
 * Described in header.
 */
bool openssl_bn_split(chunk_t chunk, BIGNUM *a, BIGNUM *b)
{
	int len;

	if ((chunk.len % 2) != 0)
	{
		return FALSE;
	}

	len = chunk.len / 2;

	if (!BN_bin2bn(chunk.ptr, len, a) ||
		!BN_bin2bn(chunk.ptr + len, len, b))
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * Described in header.
 */
bool openssl_bn2chunk(const BIGNUM *bn, chunk_t *chunk)
{
	*chunk = chunk_alloc(BN_num_bytes(bn));
	if (BN_bn2bin(bn, chunk->ptr) == chunk->len)
	{
		if (chunk->len && chunk->ptr[0] & 0x80)
		{	/* if MSB is set, prepend a zero to make it non-negative */
			*chunk = chunk_cat("cm", chunk_from_chars(0x00), *chunk);
		}
		return TRUE;
	}
	chunk_free(chunk);
	return FALSE;
}

/**
 * Described in header.
 */
chunk_t openssl_asn1_obj2chunk(const ASN1_OBJECT *asn1)
{
	if (asn1)
	{
		return chunk_create((u_char*)OBJ_get0_data(asn1), OBJ_length(asn1));
	}
	return chunk_empty;
}

/**
 * Described in header.
 */
chunk_t openssl_asn1_str2chunk(const ASN1_STRING *asn1)
{
	if (asn1)
	{
		return chunk_create((u_char*)ASN1_STRING_get0_data(asn1),
							ASN1_STRING_length(asn1));
	}
	return chunk_empty;
}

/**
 * Convert a X509 name to a ID_DER_ASN1_DN identification_t
 */
identification_t *openssl_x509_name2id(X509_NAME *name)
{
	if (name)
	{
		identification_t *id;
		chunk_t chunk;

		chunk = openssl_i2chunk(X509_NAME, name);
		if (chunk.len)
		{
			id = identification_create_from_encoding(ID_DER_ASN1_DN, chunk);
			free(chunk.ptr);
			return id;
		}
	}
	return NULL;
}

/**
 * We can't include <asn1/asn1.h>, as the ASN1_ definitions would clash
 * with OpenSSL. Redeclare what we need.
 */
int asn1_known_oid(chunk_t);
time_t asn1_to_time(chunk_t *,int);

/**
 * Described in header.
 */
int openssl_asn1_known_oid(const ASN1_OBJECT *obj)
{
	return asn1_known_oid(openssl_asn1_obj2chunk(obj));
}

/**
 * Described in header.
 */
time_t openssl_asn1_to_time(const ASN1_TIME *time)
{
	chunk_t chunk;

	if (time)
	{
		chunk = openssl_asn1_str2chunk(time);
		switch (time->type)
		{
			case V_ASN1_UTCTIME:
			case V_ASN1_GENERALIZEDTIME:
				return asn1_to_time(&chunk, time->type);
			default:
				break;
		}
	}
	DBG1(DBG_LIB, "invalid ASN1 time");
	return 0;
}
