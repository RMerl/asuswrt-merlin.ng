/*
 * Copyright (C) 2013 Andreas Steffen
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

#include "dnskey_encoder.h"

#include <utils/debug.h>

/**
 * Encode an RSA public key in DNSKEY format (RFC 3110)
 */
static bool build_pub(chunk_t *encoding, va_list args)
{
	chunk_t n, e, pubkey;
	size_t exp_len;
	u_char *pos;

	if (cred_encoding_args(args, CRED_PART_RSA_MODULUS, &n,
						   CRED_PART_RSA_PUB_EXP, &e, CRED_PART_END))
	{
		/* remove leading zeros in exponent and modulus */
		while (*e.ptr == 0)
		{
			e = chunk_skip(e, 1);
		}
		while (*n.ptr == 0)
		{
			n = chunk_skip(n, 1);
		}

		if (e.len < 256)
		{
			/* exponent length fits into a single octet */
			exp_len = 1;
			pubkey = chunk_alloc(exp_len + e.len + n.len);
			pubkey.ptr[0] = (char)e.len;
		}
		else if (e.len < 65536)
		{
			/* exponent length fits into two octets preceded by zero octet */
			exp_len = 3;
			pubkey = chunk_alloc(exp_len + e.len + n.len);
			pubkey.ptr[0] = 0x00;
			htoun16(pubkey.ptr + 1, e.len);
		}
		else
		{
			/* exponent length is too large */
			return FALSE;
		}

		/* copy exponent and modulus and convert to base64 format */
		pos = pubkey.ptr + exp_len;
		memcpy(pos, e.ptr, e.len);
		pos += e.len;
		memcpy(pos, n.ptr, n.len);
		*encoding = chunk_to_base64(pubkey, NULL);
		chunk_free(&pubkey);

		return TRUE;
	}
	return FALSE;
}

/**
 * See header.
 */
bool dnskey_encoder_encode(cred_encoding_type_t type, chunk_t *encoding,
						   va_list args)
{
	switch (type)
	{
		case PUBKEY_DNSKEY:
			return build_pub(encoding, args);
		default:
			return FALSE;
	}
}


