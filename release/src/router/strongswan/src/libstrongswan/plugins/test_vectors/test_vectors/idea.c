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
 * All testvectors from https://www.cosic.esat.kuleuven.be/nessie/testvectors/
 */

/**
 * IDEA 128 bit: set 8, vector #0
 */
crypter_test_vector_t idea1 = {
	.alg = ENCR_IDEA, .key_size = 16, .len = 8,
	.key	= "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\xDB\x2D\x4A\x92\xAA\x68\x27\x3F",
	.cipher	= "\x00\x11\x22\x33\x44\x55\x66\x77"
};

/**
 * IDEA 128 bit: set 8, vector #1
 */
crypter_test_vector_t idea2 = {
	.alg = ENCR_IDEA, .key_size = 16, .len = 8,
	.key	= "\x2B\xD6\x45\x9F\x82\xC5\xB3\x00\x95\x2C\x49\x10\x48\x81\xFF\x48",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\xF1\x29\xA6\x60\x1E\xF6\x2A\x47",
	.cipher	= "\xEA\x02\x47\x14\xAD\x5C\x4D\x84"
};


