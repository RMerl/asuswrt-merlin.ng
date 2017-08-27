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

#include "signer.h"

ENUM_BEGIN(integrity_algorithm_names, AUTH_UNDEFINED, AUTH_CAMELLIA_XCBC_96,
	"UNDEFINED",
	"HMAC_SHA1_128",
	"HMAC_SHA2_256_96",
	"HMAC_SHA2_256_256",
	"HMAC_SHA2_384_384",
	"HMAC_SHA2_512_512",
	"CAMELLIA_XCBC_96");
ENUM_NEXT(integrity_algorithm_names, AUTH_HMAC_MD5_96, AUTH_HMAC_SHA2_512_256, AUTH_CAMELLIA_XCBC_96,
	"HMAC_MD5_96",
	"HMAC_SHA1_96",
	"DES_MAC",
	"KPDK_MD5",
	"AES_XCBC_96",
	"HMAC_MD5_128",
	"HMAC_SHA1_160",
	"AES_CMAC_96",
	"AES_128_GMAC",
	"AES_192_GMAC",
	"AES_256_GMAC",
	"HMAC_SHA2_256_128",
	"HMAC_SHA2_384_192",
	"HMAC_SHA2_512_256");
ENUM_END(integrity_algorithm_names, AUTH_HMAC_SHA2_512_256);

