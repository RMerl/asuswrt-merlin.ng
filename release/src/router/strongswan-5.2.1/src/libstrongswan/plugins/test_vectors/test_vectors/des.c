/*
 * Copyright (C) 2009 Andreas Steffen
 * Hochschule fuer Technik Rapperswil
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
 * DES 56 bit: set 8, vector #0
 */
crypter_test_vector_t des_ecb1 = {
	.alg = ENCR_DES_ECB, .key_size = 8, .len = 8,
	.key	= "\x00\x01\x02\x03\x04\x05\x06\x07",
	.plain	= "\x41\xAD\x06\x85\x48\x80\x9D\x02",
	.cipher	= "\x00\x11\x22\x33\x44\x55\x66\x77"
};

/**
 * DES 56 bit: set 8, vector #1
 */
crypter_test_vector_t des_ecb2 = {
	.alg = ENCR_DES_ECB, .key_size = 8, .len = 8,
	.key	= "\x2B\xD6\x45\x9F\x82\xC5\xB3\x00",
	.plain	= "\xB1\x0F\x84\x30\x97\xA0\xF9\x32",
	.cipher	= "\xEA\x02\x47\x14\xAD\x5C\x4D\x84"
};

/**
 * DES 56 bit: set 8, vector #0
 */
crypter_test_vector_t des_cbc1 = {
	.alg = ENCR_DES, .key_size = 8, .len = 8,
	.key	= "\x00\x01\x02\x03\x04\x05\x06\x07",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x41\xAD\x06\x85\x48\x80\x9D\x02",
	.cipher	= "\x00\x11\x22\x33\x44\x55\x66\x77"
};

/**
 * DES 56 bit: set 8, vector #1
 */
crypter_test_vector_t des_cbc2 = {
	.alg = ENCR_DES, .key_size = 8, .len = 8,
	.key	= "\x2B\xD6\x45\x9F\x82\xC5\xB3\x00",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\xB1\x0F\x84\x30\x97\xA0\xF9\x32",
	.cipher	= "\xEA\x02\x47\x14\xAD\x5C\x4D\x84"
};

