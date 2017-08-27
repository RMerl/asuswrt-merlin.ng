/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "prf.h"

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

