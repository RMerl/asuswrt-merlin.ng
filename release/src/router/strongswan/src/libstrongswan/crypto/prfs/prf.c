/*
 * Copyright (C) 2018 Tobias Brunner
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

#include "prf.h"

#include <asn1/oid.h>

ENUM_BEGIN(pseudo_random_function_names, PRF_UNDEFINED, PRF_CAMELLIA128_XCBC,
	"PRF_UNDEFINED",
	"PRF_FIPS_SHA1_160",
	"PRF_FIPS_DES",
	"PRF_KEYED_SHA1",
	"PRF_CAMELLIA128_XCBC");
ENUM_NEXT(pseudo_random_function_names, PRF_HMAC_MD5, PRF_AES128_CMAC, PRF_CAMELLIA128_XCBC,
	"PRF_HMAC_MD5",
	"PRF_HMAC_SHA1",
	"PRF_HMAC_TIGER",
	"PRF_AES128_XCBC",
	"PRF_HMAC_SHA2_256",
	"PRF_HMAC_SHA2_384",
	"PRF_HMAC_SHA2_512",
	"PRF_AES128_CMAC");
ENUM_END(pseudo_random_function_names, PRF_AES128_CMAC);

/*
 * Described in header.
 */
pseudo_random_function_t pseudo_random_function_from_oid(int oid)
{
	switch (oid)
	{
		case OID_HMAC_SHA1:
			return PRF_HMAC_SHA1;
		case OID_HMAC_SHA256:
			return PRF_HMAC_SHA2_256;
		case OID_HMAC_SHA384:
			return PRF_HMAC_SHA2_384;
		case OID_HMAC_SHA512:
			return PRF_HMAC_SHA2_512;
		case OID_HMAC_SHA224:
		case OID_HMAC_SHA512_224:
		case OID_HMAC_SHA512_256:
		default:
			return PRF_UNDEFINED;
	}
}
