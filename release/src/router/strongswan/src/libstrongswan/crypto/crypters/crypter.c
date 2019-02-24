/*
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

#include <asn1/oid.h>

#include "crypter.h"

ENUM_BEGIN(encryption_algorithm_names, ENCR_DES_IV64, ENCR_DES_IV32,
	"DES_IV64",
	"DES_CBC",
	"3DES_CBC",
	"RC5_CBC",
	"IDEA_CBC",
	"CAST_CBC",
	"BLOWFISH_CBC",
	"3IDEA",
	"DES_IV32");
ENUM_NEXT(encryption_algorithm_names, ENCR_NULL, ENCR_AES_CCM_ICV16, ENCR_DES_IV32,
	"NULL",
	"AES_CBC",
	"AES_CTR",
	"AES_CCM_8",
	"AES_CCM_12",
	"AES_CCM_16");
ENUM_NEXT(encryption_algorithm_names, ENCR_AES_GCM_ICV8, ENCR_NULL_AUTH_AES_GMAC, ENCR_AES_CCM_ICV16,
	"AES_GCM_8",
	"AES_GCM_12",
	"AES_GCM_16",
	"NULL_AES_GMAC");
ENUM_NEXT(encryption_algorithm_names, ENCR_CAMELLIA_CBC, ENCR_CHACHA20_POLY1305, ENCR_NULL_AUTH_AES_GMAC,
	"CAMELLIA_CBC",
	"CAMELLIA_CTR",
	"CAMELLIA_CCM_8",
	"CAMELLIA_CCM_12",
	"CAMELLIA_CCM_16",
	"CHACHA20_POLY1305");
ENUM_NEXT(encryption_algorithm_names, ENCR_UNDEFINED, ENCR_RC2_CBC, ENCR_CHACHA20_POLY1305,
	"UNDEFINED",
	"DES_ECB",
	"SERPENT_CBC",
	"TWOFISH_CBC",
	"RC2_CBC");
ENUM_END(encryption_algorithm_names, ENCR_RC2_CBC);

/*
 * Described in header.
 */
encryption_algorithm_t encryption_algorithm_from_oid(int oid, size_t *key_size)
{
	encryption_algorithm_t alg;
	size_t alg_key_size;

	switch (oid)
	{
		case OID_DES_CBC:
			alg = ENCR_DES;
			alg_key_size = 0;
			break;
		case OID_3DES_EDE_CBC:
			alg = ENCR_3DES;
			alg_key_size = 0;
			break;
		case OID_AES128_CBC:
			alg = ENCR_AES_CBC;
			alg_key_size = 128;
			break;
		case OID_AES192_CBC:
			alg = ENCR_AES_CBC;
			alg_key_size = 192;
			break;
		case OID_AES256_CBC:
			alg = ENCR_AES_CBC;
			alg_key_size = 256;
			break;
		case OID_CAMELLIA128_CBC:
			alg = ENCR_CAMELLIA_CBC;
			alg_key_size = 128;
			break;
		case OID_CAMELLIA192_CBC:
			alg = ENCR_CAMELLIA_CBC;
			alg_key_size = 192;
			break;
		case OID_CAMELLIA256_CBC:
			alg = ENCR_CAMELLIA_CBC;
			alg_key_size = 256;
			break;
		case OID_BLOWFISH_CBC:
			alg = ENCR_BLOWFISH;
			alg_key_size = 0;
			break;
		default:
			alg = ENCR_UNDEFINED;
			alg_key_size = 0;
	}
	if (key_size)
	{
			*key_size = alg_key_size;
	}
	return alg;
}

/*
 * Described in header.
 */
int encryption_algorithm_to_oid(encryption_algorithm_t alg, size_t key_size)
{
	int oid;

	switch(alg)
	{
		case ENCR_DES:
			oid = OID_DES_CBC;
			break;
		case ENCR_3DES:
			oid = OID_3DES_EDE_CBC;
			break;
		case ENCR_AES_CBC:
			switch (key_size)
			{
				case 128:
					oid = OID_AES128_CBC;
					break;
				case 192:
					oid = OID_AES192_CBC;
					break;
				case 256:
					oid = OID_AES256_CBC;
					break;
				default:
					oid = OID_UNKNOWN;
			}
			break;
		case ENCR_CAMELLIA_CBC:
			switch (key_size)
			{
				case 128:
					oid = OID_CAMELLIA128_CBC;
					break;
				case 192:
					oid = OID_CAMELLIA192_CBC;
					break;
				case 256:
					oid = OID_CAMELLIA256_CBC;
					break;
				default:
					oid = OID_UNKNOWN;
			}
			break;
		case ENCR_BLOWFISH:
			oid = OID_BLOWFISH_CBC;
			break;
		default:
			oid = OID_UNKNOWN;
	}
	return oid;
}

/*
 * Described in header.
 */
bool encryption_algorithm_is_aead(encryption_algorithm_t alg)
{
	switch (alg)
	{
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
		case ENCR_NULL_AUTH_AES_GMAC:
		case ENCR_CAMELLIA_CCM_ICV8:
		case ENCR_CAMELLIA_CCM_ICV12:
		case ENCR_CAMELLIA_CCM_ICV16:
		case ENCR_CHACHA20_POLY1305:
			return TRUE;
		default:
			return FALSE;
	}
}
