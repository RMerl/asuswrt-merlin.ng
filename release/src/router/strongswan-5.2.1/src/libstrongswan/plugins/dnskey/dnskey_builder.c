/*
 * Copyright (C) 2009 Martin Willi
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

#include "dnskey_builder.h"

#include <utils/debug.h>
#include <credentials/keys/private_key.h>


typedef struct dnskey_rr_t dnskey_rr_t;
typedef enum dnskey_algorithm_t dnskey_algorithm_t;

/**
 * Header of a DNSKEY resource record
 */
struct dnskey_rr_t {
	u_int16_t flags;
	u_int8_t protocol;
	u_int8_t algorithm;
	u_int8_t data[];
} __attribute__((__packed__));

/**
 * DNSSEC algorithms, RFC4034 Appendix A.1.
 */
enum dnskey_algorithm_t {
	DNSKEY_ALG_RSA_MD5 = 1,
	DNSKEY_ALG_DH = 2,
	DNSKEY_ALG_DSA = 3,
	DNSKEY_ALG_RSA_SHA1 = 5,
	DNSKEY_ALG_DSA_NSEC3_SHA1 = 6,
	DNSKEY_ALG_RSA_SHA1_NSEC3_SHA1 = 7,
	DNSKEY_ALG_RSA_SHA256 = 8,
	DNSKEY_ALG_RSA_SHA512 = 10,
	DNSKEY_ALG_ECC_GOST = 12,
	DNSKEY_ALG_ECDSA_P256_SHA256 = 13,
	DNSKEY_ALG_ECDSA_P384_SHA384 = 14
};

/**
 * Load a generic public key from a DNSKEY RR blob
 */
static dnskey_public_key_t *parse_public_key(chunk_t blob)
{
	dnskey_rr_t *rr = (dnskey_rr_t*)blob.ptr;

	if (blob.len < sizeof(dnskey_rr_t))
	{
		DBG1(DBG_LIB, "DNSKEY too short");
		return NULL;
	}
	blob = chunk_skip(blob, sizeof(dnskey_rr_t));

	switch (rr->algorithm)
	{
		case DNSKEY_ALG_RSA_MD5:
		case DNSKEY_ALG_RSA_SHA1:
		case DNSKEY_ALG_RSA_SHA1_NSEC3_SHA1:
		case DNSKEY_ALG_RSA_SHA256:
		case DNSKEY_ALG_RSA_SHA512:
			return lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
									  BUILD_BLOB_DNSKEY, blob, BUILD_END);
		default:
			DBG1(DBG_LIB, "DNSKEY public key algorithm %d not supported",
				 rr->algorithm);
			return NULL;
	}
}

/**
 * Load a RSA public key from DNSKEY RR data
 */
static dnskey_public_key_t *parse_rsa_public_key(chunk_t blob)
{
	chunk_t n, e;

	if (blob.len < 3)
	{
		DBG1(DBG_LIB, "RFC 3110 public key blob too short for exponent length");
		return NULL;
	}

	if (blob.ptr[0])
	{
		e.len = blob.ptr[0];
		blob = chunk_skip(blob, 1);
	}
	else
	{
		e.len = blob.ptr[1] * 256 + blob.ptr[2];
		blob = chunk_skip(blob, 3);
	}
	e.ptr = blob.ptr;
	if (e.len >= blob.len)
	{
		DBG1(DBG_LIB, "RFC 3110 public key blob too short for exponent");
		return NULL;
	}
	n = chunk_skip(blob, e.len);

	return lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
						BUILD_RSA_MODULUS, n, BUILD_RSA_PUB_EXP, e,
						BUILD_END);
}

/**
 * See header.
 */
dnskey_public_key_t *dnskey_public_key_load(key_type_t type, va_list args)
{
	chunk_t blob = chunk_empty;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_DNSKEY:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (!blob.ptr)
	{
		return NULL;
	}
	switch (type)
	{
		case KEY_ANY:
			return parse_public_key(blob);
		case KEY_RSA:
			return parse_rsa_public_key(blob);
		default:
			return NULL;
	}
}

