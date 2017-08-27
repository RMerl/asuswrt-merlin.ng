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

#include "pgp_encoder.h"

#include <utils/debug.h>

/**
 * Build a PGPv3 fingerprint
 */
static bool build_v3_fingerprint(chunk_t *encoding, va_list args)
{
	hasher_t *hasher;
	chunk_t n, e;

	if (cred_encoding_args(args, CRED_PART_RSA_MODULUS, &n,
						   CRED_PART_RSA_PUB_EXP, &e, CRED_PART_END))
	{
		hasher = lib->crypto->create_hasher(lib->crypto, HASH_MD5);
		if (!hasher)
		{
			DBG1(DBG_LIB, "MD5 hash algorithm not supported, PGP"
				 " fingerprinting failed");
			return FALSE;
		}
		/* remove leading zero bytes before hashing modulus and exponent */
		while (n.len > 0 && n.ptr[0] == 0x00)
		{
			n = chunk_skip(n, 1);
		}
		while (e.len > 0 && e.ptr[0] == 0x00)
		{
			e = chunk_skip(e, 1);
		}
		if (!hasher->allocate_hash(hasher, n, NULL) ||
			!hasher->allocate_hash(hasher, e, encoding))
		{
			hasher->destroy(hasher);
			return FALSE;
		}
		hasher->destroy(hasher);
		return TRUE;
	}
	return FALSE;
}

/**
 * See header.
 */
bool pgp_encoder_encode(cred_encoding_type_t type, chunk_t *encoding,
						va_list args)
{
	switch (type)
	{
		case KEYID_PGPV3:
			return build_v3_fingerprint(encoding, args);
		default:
			return FALSE;
	}
}

