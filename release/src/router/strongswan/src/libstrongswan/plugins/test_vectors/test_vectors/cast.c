/*
 * Copyright (C) 2009 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the Licenseor (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be usefulbut
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <crypto/crypto_tester.h>

/**
 * Test from RFC 2144
 */
crypter_test_vector_t cast1 = {
	.alg = ENCR_CAST, .key_size = 16, .len = 8,
	.key    = "\x01\x23\x45\x67\x12\x34\x56\x78\x23\x45\x67\x89\x34\x56\x78\x9A",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x01\x23\x45\x67\x89\xAB\xCD\xEF",
	.cipher	= "\x23\x8B\x4F\xE5\x84\x7E\x44\xB2"
};

