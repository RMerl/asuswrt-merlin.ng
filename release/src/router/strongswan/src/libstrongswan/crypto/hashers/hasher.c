/*
 * Copyright (C) 2012-2015 Tobias Brunner
 * Copyright (C) 2015-2017 Andreas Steffen
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * HSR Hochschule fuer Technik Rapperswil
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

#include "hasher.h"

#include <asn1/oid.h>
#include <credentials/keys/signature_params.h>

ENUM_BEGIN(hash_algorithm_names, HASH_SHA1, HASH_IDENTITY,
	"HASH_SHA1",
	"HASH_SHA2_256",
	"HASH_SHA2_384",
	"HASH_SHA2_512",
	"HASH_IDENTITY");
ENUM_NEXT(hash_algorithm_names, HASH_UNKNOWN, HASH_SHA3_512, HASH_IDENTITY,
	"HASH_UNKNOWN",
	"HASH_MD2",
	"HASH_MD4",
	"HASH_MD5",
	"HASH_SHA2_224",
	"HASH_SHA3_224",
	"HASH_SHA3_256",
	"HASH_SHA3_384",
	"HASH_SHA3_512");
ENUM_END(hash_algorithm_names, HASH_SHA3_512);

ENUM_BEGIN(hash_algorithm_short_names, HASH_SHA1, HASH_IDENTITY,
	"sha1",
	"sha256",
	"sha384",
	"sha512",
	"identity");
ENUM_NEXT(hash_algorithm_short_names, HASH_UNKNOWN, HASH_SHA3_512, HASH_IDENTITY,
	"unknown",
	"md2",
	"md4",
	"md5",
	"sha224",
	"sha3_224",
	"sha3_256",
	"sha3_384",
	"sha3_512");
ENUM_END(hash_algorithm_short_names, HASH_SHA3_512);

ENUM_BEGIN(hash_algorithm_short_names_upper, HASH_SHA1, HASH_IDENTITY,
	"SHA1",
	"SHA2_256",
	"SHA2_384",
	"SHA2_512",
	"IDENTITY");
ENUM_NEXT(hash_algorithm_short_names_upper, HASH_UNKNOWN, HASH_SHA3_512, HASH_IDENTITY,
	"UNKNOWN",
	"MD2",
	"MD4",
	"MD5",
	"SHA2_224",
	"SHA3_224",
	"SHA3_256",
	"SHA3_384",
	"SHA3_512");
ENUM_END(hash_algorithm_short_names_upper, HASH_SHA3_512);

/*
 * Described in header
 */
size_t hasher_hash_size(hash_algorithm_t alg)
{
	switch (alg)
	{
		case HASH_SHA1:
			return HASH_SIZE_SHA1;
		case HASH_SHA256:
			return HASH_SIZE_SHA256;
		case HASH_SHA384:
			return HASH_SIZE_SHA384;
		case HASH_SHA512:
			return HASH_SIZE_SHA512;
		case HASH_MD2:
			return HASH_SIZE_MD2;
		case HASH_MD4:
			return HASH_SIZE_MD4;
		case HASH_MD5:
			return HASH_SIZE_MD5;
		case HASH_SHA224:
			return HASH_SIZE_SHA224;
		case HASH_SHA3_224:
			return HASH_SIZE_SHA224;
		case HASH_SHA3_256:
			return HASH_SIZE_SHA256;
		case HASH_SHA3_384:
			return HASH_SIZE_SHA384;
		case HASH_SHA3_512:
			return HASH_SIZE_SHA512;
		case HASH_IDENTITY:
		case HASH_UNKNOWN:
			break;
	}
	return 0;
}

/*
 * Described in header.
 */
hash_algorithm_t hasher_algorithm_from_oid(int oid)
{
	switch (oid)
	{
		case OID_MD2:
		case OID_MD2_WITH_RSA:
			return HASH_MD2;
		case OID_MD5:
		case OID_MD5_WITH_RSA:
			return HASH_MD5;
		case OID_SHA1:
		case OID_SHA1_WITH_RSA:
			return HASH_SHA1;
		case OID_SHA224:
		case OID_SHA224_WITH_RSA:
			return HASH_SHA224;
		case OID_SHA256:
		case OID_SHA256_WITH_RSA:
			return HASH_SHA256;
		case OID_SHA384:
		case OID_SHA384_WITH_RSA:
			return HASH_SHA384;
		case OID_SHA512:
		case OID_SHA512_WITH_RSA:
			return HASH_SHA512;
		case OID_SHA3_224:
		case OID_RSASSA_PKCS1V15_WITH_SHA3_224:
			return HASH_SHA3_224;
		case OID_SHA3_256:
		case OID_RSASSA_PKCS1V15_WITH_SHA3_256:
			return HASH_SHA3_256;
		case OID_SHA3_384:
		case OID_RSASSA_PKCS1V15_WITH_SHA3_384:
			return HASH_SHA3_384;
		case OID_SHA3_512:
		case OID_RSASSA_PKCS1V15_WITH_SHA3_512:
			return HASH_SHA3_512;
		case OID_ED25519:
		case OID_ED448:
			return HASH_IDENTITY;
		default:
			return HASH_UNKNOWN;
	}
}

/*
 * Described in header.
 */
hash_algorithm_t hasher_algorithm_from_prf(pseudo_random_function_t alg)
{
	switch (alg)
	{
		case PRF_HMAC_MD5:
			return HASH_MD5;
		case PRF_HMAC_SHA1:
		case PRF_FIPS_SHA1_160:
		case PRF_KEYED_SHA1:
			return HASH_SHA1;
		case PRF_HMAC_SHA2_256:
			return HASH_SHA256;
		case PRF_HMAC_SHA2_384:
			return HASH_SHA384;
		case PRF_HMAC_SHA2_512:
			return HASH_SHA512;
		case PRF_HMAC_TIGER:
		case PRF_AES128_XCBC:
		case PRF_AES128_CMAC:
		case PRF_FIPS_DES:
		case PRF_CAMELLIA128_XCBC:
		case PRF_UNDEFINED:
			break;
	}
	return HASH_UNKNOWN;
}

/*
 * Described in header.
 */
hash_algorithm_t hasher_algorithm_from_integrity(integrity_algorithm_t alg,
												 size_t *length)
{
	if (length)
	{
		switch (alg)
		{
			case AUTH_HMAC_MD5_96:
			case AUTH_HMAC_SHA1_96:
			case AUTH_HMAC_SHA2_256_96:
				*length = 12;
				break;
			case AUTH_HMAC_MD5_128:
			case AUTH_HMAC_SHA1_128:
			case AUTH_HMAC_SHA2_256_128:
				*length = 16;
				break;
			case AUTH_HMAC_SHA1_160:
				*length = 20;
				break;
			case AUTH_HMAC_SHA2_384_192:
				*length = 24;
				break;
			case AUTH_HMAC_SHA2_256_256:
			case AUTH_HMAC_SHA2_512_256:
				*length = 32;
				break;
			case AUTH_HMAC_SHA2_384_384:
				*length = 48;
				break;
			case AUTH_HMAC_SHA2_512_512:
				*length = 64;
				break;
			default:
				break;
		}
	}
	switch (alg)
	{
		case AUTH_HMAC_MD5_96:
		case AUTH_HMAC_MD5_128:
		case AUTH_KPDK_MD5:
			return HASH_MD5;
		case AUTH_HMAC_SHA1_96:
		case AUTH_HMAC_SHA1_128:
		case AUTH_HMAC_SHA1_160:
			return HASH_SHA1;
		case AUTH_HMAC_SHA2_256_96:
		case AUTH_HMAC_SHA2_256_128:
		case AUTH_HMAC_SHA2_256_256:
			return HASH_SHA256;
		case AUTH_HMAC_SHA2_384_192:
		case AUTH_HMAC_SHA2_384_384:
			return HASH_SHA384;
		case AUTH_HMAC_SHA2_512_256:
		case AUTH_HMAC_SHA2_512_512:
			return HASH_SHA512;
		case AUTH_AES_CMAC_96:
		case AUTH_AES_128_GMAC:
		case AUTH_AES_192_GMAC:
		case AUTH_AES_256_GMAC:
		case AUTH_AES_XCBC_96:
		case AUTH_DES_MAC:
		case AUTH_CAMELLIA_XCBC_96:
		case AUTH_UNDEFINED:
			break;
	}
	return HASH_UNKNOWN;
}

/*
 * Described in header.
 */
integrity_algorithm_t hasher_algorithm_to_integrity(hash_algorithm_t alg,
													size_t length)
{
	switch (alg)
	{
		case HASH_MD5:
			switch (length)
			{
				case 12:
					return AUTH_HMAC_MD5_96;
				case 16:
					return AUTH_HMAC_MD5_128;
			}
			break;
		case HASH_SHA1:
			switch (length)
			{
				case 12:
					return AUTH_HMAC_SHA1_96;
				case 16:
					return AUTH_HMAC_SHA1_128;
				case 20:
					return AUTH_HMAC_SHA1_160;
			}
			break;
		case HASH_SHA256:
			switch (length)
			{
				case 12:
					return AUTH_HMAC_SHA2_256_96;
				case 16:
					return AUTH_HMAC_SHA2_256_128;
				case 32:
					return AUTH_HMAC_SHA2_256_256;
			}
			break;
		case HASH_SHA384:
			switch (length)
			{
				case 24:
					return AUTH_HMAC_SHA2_384_192;
				case 48:
					return AUTH_HMAC_SHA2_384_384;

			}
			break;
		case HASH_SHA512:
			switch (length)
			{
				case 32:
					return AUTH_HMAC_SHA2_512_256;
				case 64:
					return AUTH_HMAC_SHA2_512_512;
			}
			break;
		case HASH_MD2:
		case HASH_MD4:
		case HASH_SHA224:
		case HASH_SHA3_224:
		case HASH_SHA3_256:
		case HASH_SHA3_384:
		case HASH_SHA3_512:
		case HASH_IDENTITY:
		case HASH_UNKNOWN:
			break;
	}
	return AUTH_UNDEFINED;
}

/*
 * Described in header.
 */
bool hasher_algorithm_for_ikev2(hash_algorithm_t alg)
{
	switch (alg)
	{
		case HASH_IDENTITY:
		case HASH_SHA256:
		case HASH_SHA384:
		case HASH_SHA512:
			return TRUE;
		case HASH_UNKNOWN:
		case HASH_MD2:
		case HASH_MD4:
		case HASH_MD5:
		case HASH_SHA1:
		case HASH_SHA224:
		case HASH_SHA3_224:
		case HASH_SHA3_256:
		case HASH_SHA3_384:
		case HASH_SHA3_512:
			break;
	}
	return FALSE;
}

/*
 * Described in header.
 */
int hasher_algorithm_to_oid(hash_algorithm_t alg)
{
	int oid;

	switch (alg)
	{
		case HASH_MD2:
			oid = OID_MD2;
			break;
		case HASH_MD5:
			oid = OID_MD5;
			break;
		case HASH_SHA1:
			oid = OID_SHA1;
			break;
		case HASH_SHA224:
			oid = OID_SHA224;
			break;
		case HASH_SHA256:
			oid = OID_SHA256;
			break;
		case HASH_SHA384:
			oid = OID_SHA384;
			break;
		case HASH_SHA512:
			oid = OID_SHA512;
			break;
		case HASH_SHA3_224:
			oid = OID_SHA3_224;
			break;
		case HASH_SHA3_256:
			oid = OID_SHA3_256;
			break;
		case HASH_SHA3_384:
			oid = OID_SHA3_384;
			break;
		case HASH_SHA3_512:
			oid = OID_SHA3_512;
			break;
		default:
			oid = OID_UNKNOWN;
	}
	return oid;
}

/*
 * Described in header.
 */
int hasher_signature_algorithm_to_oid(hash_algorithm_t alg, key_type_t key)
{
	switch (key)
	{
		case KEY_RSA:
			switch (alg)
			{
				case HASH_MD2:
					return OID_MD2_WITH_RSA;
				case HASH_MD5:
					return OID_MD5_WITH_RSA;
				case HASH_SHA1:
					return OID_SHA1_WITH_RSA;
				case HASH_SHA224:
					return OID_SHA224_WITH_RSA;
				case HASH_SHA256:
					return OID_SHA256_WITH_RSA;
				case HASH_SHA384:
					return OID_SHA384_WITH_RSA;
				case HASH_SHA512:
					return OID_SHA512_WITH_RSA;
				case HASH_SHA3_224:
					return OID_RSASSA_PKCS1V15_WITH_SHA3_224;
				case HASH_SHA3_256:
					return OID_RSASSA_PKCS1V15_WITH_SHA3_256;
				case HASH_SHA3_384:
					return OID_RSASSA_PKCS1V15_WITH_SHA3_384;
				case HASH_SHA3_512:
					return OID_RSASSA_PKCS1V15_WITH_SHA3_512;
				default:
					return OID_UNKNOWN;
			}
		case KEY_ECDSA:
			switch (alg)
			{
				case HASH_SHA1:
					return OID_ECDSA_WITH_SHA1;
				case HASH_SHA256:
					return OID_ECDSA_WITH_SHA256;
				case HASH_SHA384:
					return OID_ECDSA_WITH_SHA384;
				case HASH_SHA512:
					return OID_ECDSA_WITH_SHA512;
				default:
					return OID_UNKNOWN;
			}
		case KEY_ED25519:
			switch (alg)
			{
				case HASH_IDENTITY:
					return OID_ED25519;
				default:
					return OID_UNKNOWN;
			}
		case KEY_ED448:
			switch (alg)
			{
				case HASH_IDENTITY:
					return OID_ED448;
				default:
					return OID_UNKNOWN;
			}
		case KEY_BLISS:
			switch (alg)
			{
				case HASH_SHA256:
					return OID_BLISS_WITH_SHA2_256;
				case HASH_SHA384:
					return OID_BLISS_WITH_SHA2_384;
				case HASH_SHA512:
					return OID_BLISS_WITH_SHA2_512;
				case HASH_SHA3_256:
					return OID_BLISS_WITH_SHA3_256;
				case HASH_SHA3_384:
					return OID_BLISS_WITH_SHA3_384;
				case HASH_SHA3_512:
					return OID_BLISS_WITH_SHA3_512;
				default:
					return OID_UNKNOWN;
			}
		default:
			return OID_UNKNOWN;
	}
}

/*
 * Defined in header.
 */
hash_algorithm_t hasher_from_signature_scheme(signature_scheme_t scheme,
											  void *params)
{
	switch (scheme)
	{
		case SIGN_UNKNOWN:
		case SIGN_RSA_EMSA_PKCS1_NULL:
		case SIGN_ECDSA_WITH_NULL:
			break;
		case SIGN_RSA_EMSA_PSS:
			if (params)
			{
				rsa_pss_params_t *pss = params;
				return pss->hash;
			}
			break;
		case SIGN_ED25519:
		case SIGN_ED448:
			return HASH_IDENTITY;
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return HASH_MD5;
		case SIGN_RSA_EMSA_PKCS1_SHA1:
		case SIGN_ECDSA_WITH_SHA1_DER:
			return HASH_SHA1;
		case SIGN_RSA_EMSA_PKCS1_SHA2_224:
			return HASH_SHA224;
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
		case SIGN_ECDSA_WITH_SHA256_DER:
		case SIGN_ECDSA_256:
		case SIGN_BLISS_WITH_SHA2_256:
			return HASH_SHA256;
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
		case SIGN_ECDSA_WITH_SHA384_DER:
		case SIGN_ECDSA_384:
		case SIGN_BLISS_WITH_SHA2_384:
			return HASH_SHA384;
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
		case SIGN_ECDSA_WITH_SHA512_DER:
		case SIGN_ECDSA_521:
		case SIGN_BLISS_WITH_SHA2_512:
			return HASH_SHA512;
		case SIGN_RSA_EMSA_PKCS1_SHA3_224:
			return HASH_SHA3_224;
		case SIGN_RSA_EMSA_PKCS1_SHA3_256:
		case SIGN_BLISS_WITH_SHA3_256:
			return HASH_SHA3_256;
		case SIGN_RSA_EMSA_PKCS1_SHA3_384:
		case SIGN_BLISS_WITH_SHA3_384:
			return HASH_SHA3_384;
		case SIGN_RSA_EMSA_PKCS1_SHA3_512:
		case SIGN_BLISS_WITH_SHA3_512:
			return HASH_SHA3_512;
	}
	return HASH_UNKNOWN;
}
