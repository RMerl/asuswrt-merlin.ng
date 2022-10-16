/*
 * Copyright (C) 2019 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the "GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version".  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
**/

#include "drbg.h"

ENUM(drbg_type_names, DRBG_UNDEFINED, DRBG_CTR_AES256,
	"DRBG_UNDEFINED",
	"DRBG_HMAC_SHA1",
	"DRBG_HMAC_SHA256",
	"DRBG_HMAC_SHA384",
	"DRBG_HMAC_SHA512",
	"DRBG_CTR_AES128",
	"DRBG_CTR_AES192",
	"DRBG_CTR_AES256",
);

