/*
 * Copyright (C) 2011 Tobias Brunner
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

#include "keymat.h"

#include <sa/ikev1/keymat_v1.h>
#include <sa/ikev2/keymat_v2.h>

static keymat_constructor_t keymat_v1_ctor = NULL, keymat_v2_ctor = NULL;

/**
 * See header
 */
keymat_t *keymat_create(ike_version_t version, bool initiator)
{
	keymat_t *keymat = NULL;

	switch (version)
	{
		case IKEV1:
#ifdef USE_IKEV1
			keymat = keymat_v1_ctor ? keymat_v1_ctor(initiator)
									: &keymat_v1_create(initiator)->keymat;
#endif
			break;
		case IKEV2:
#ifdef USE_IKEV2
			keymat = keymat_v2_ctor ? keymat_v2_ctor(initiator)
									: &keymat_v2_create(initiator)->keymat;
#endif
			break;
		default:
			break;
	}
	return keymat;
}

/**
 * Implicit key length for an algorithm
 */
typedef struct {
	/** IKEv2 algorithm identifier */
	int alg;
	/** key length in bits */
	int len;
} keylen_entry_t;

/**
 * See header.
 */
int keymat_get_keylen_encr(encryption_algorithm_t alg)
{
	keylen_entry_t map[] = {
		{ENCR_DES,					 64},
		{ENCR_3DES,					192},
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (map[i].alg == alg)
		{
			return map[i].len;
		}
	}
	return 0;
}

/**
 * See header.
 */
int keymat_get_keylen_integ(integrity_algorithm_t alg)
{
	keylen_entry_t map[] = {
		{AUTH_HMAC_MD5_96,			128},
		{AUTH_HMAC_MD5_128,			128},
		{AUTH_HMAC_SHA1_96,			160},
		{AUTH_HMAC_SHA1_160,		160},
		{AUTH_HMAC_SHA2_256_96,		256},
		{AUTH_HMAC_SHA2_256_128,	256},
		{AUTH_HMAC_SHA2_384_192,	384},
		{AUTH_HMAC_SHA2_512_256,	512},
		{AUTH_AES_XCBC_96,			128},
		{AUTH_AES_CMAC_96,			128},
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (map[i].alg == alg)
		{
			return map[i].len;
		}
	}
	return 0;
}

/**
 * See header.
 */
void keymat_register_constructor(ike_version_t version,
								 keymat_constructor_t create)
{
	switch (version)
	{
		case IKEV1:
			keymat_v1_ctor = create;
			break;
		case IKEV2:
			keymat_v2_ctor = create;
			break;
		default:
			break;
	}
}
