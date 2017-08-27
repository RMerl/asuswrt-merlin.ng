/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
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

#include "openssl_util.h"

#include <utils/debug.h>

#include <openssl/evp.h>
#include <openssl/x509.h>

/**
 * Described in header.
 */
bool openssl_hash_chunk(int hash_type, chunk_t data, chunk_t *hash)
{
	EVP_MD_CTX *ctx;
	bool ret = FALSE;
	const EVP_MD *hasher = EVP_get_digestbynid(hash_type);
	if (!hasher)
	{
		return FALSE;
	}

	ctx = EVP_MD_CTX_create();
	if (!ctx)
	{
		goto error;
	}

	if (!EVP_DigestInit_ex(ctx, hasher, NULL))
	{
		goto error;
	}

	if (!EVP_DigestUpdate(ctx, data.ptr, data.len))
	{
		goto error;
	}

	*hash = chunk_alloc(hasher->md_size);
	if (!EVP_DigestFinal_ex(ctx, hash->ptr, NULL))
	{
		chunk_free(hash);
		goto error;
	}

	ret = TRUE;
error:
	if (ctx)
	{
		EVP_MD_CTX_destroy(ctx);
	}
	return ret;
}

/**
 * Described in header.
 */
bool openssl_bn_cat(int len, BIGNUM *a, BIGNUM *b, chunk_t *chunk)
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
bool openssl_bn2chunk(BIGNUM *bn, chunk_t *chunk)
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
chunk_t openssl_asn1_obj2chunk(ASN1_OBJECT *asn1)
{
	if (asn1)
	{
		return chunk_create((u_char*)asn1->data, asn1->length);
	}
	return chunk_empty;
}

/**
 * Described in header.
 */
chunk_t openssl_asn1_str2chunk(ASN1_STRING *asn1)
{
	if (asn1)
	{
		return chunk_create(ASN1_STRING_data(asn1), ASN1_STRING_length(asn1));
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
int openssl_asn1_known_oid(ASN1_OBJECT *obj)
{
	return asn1_known_oid(openssl_asn1_obj2chunk(obj));
}

/**
 * Described in header.
 */
time_t openssl_asn1_to_time(ASN1_TIME *time)
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
