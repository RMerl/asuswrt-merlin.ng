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
 * All testvectors from http://www.schneier.com/code/ecb_ival.txt
 */

/**
 * Twofish 128 bit: I=49
 */
crypter_test_vector_t twofish_cbc1 = {
	.alg = ENCR_TWOFISH_CBC, .key_size = 16, .len = 16,
	.key	= "\xBC\xA7\x24\xA5\x45\x33\xC6\x98\x7E\x14\xAA\x82\x79\x52\xF9\x21",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x6B\x45\x92\x86\xF3\xFF\xD2\x8D\x49\xF1\x5B\x15\x81\xB0\x8E\x42",
	.cipher	= "\x5D\x9D\x4E\xEF\xFA\x91\x51\x57\x55\x24\xF1\x15\x81\x5A\x12\xE0"
};

/**
 * Twofish 192 bit: I=49
 */
crypter_test_vector_t twofish_cbc2 = {
	.alg = ENCR_TWOFISH_CBC, .key_size = 24, .len = 16,
	.key	= "\xFB\x66\x52\x2C\x33\x2F\xCC\x4C\x04\x2A\xBE\x32\xFA\x9E\x90\x2F"
			  "\xDE\xA4\xF3\xDA\x75\xEC\x7A\x8E",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\xF0\xAB\x73\x30\x11\x25\xFA\x21\xEF\x70\xBE\x53\x85\xFB\x76\xB6",
	.cipher	= "\xE7\x54\x49\x21\x2B\xEE\xF9\xF4\xA3\x90\xBD\x86\x0A\x64\x09\x41"
};

/**
 * Twofish 256 bit: I=49
 */
crypter_test_vector_t twofish_cbc3 = {
	.alg = ENCR_TWOFISH_CBC, .key_size = 32, .len = 16,
	.key	= "\x24\x8A\x7F\x35\x28\xB1\x68\xAC\xFD\xD1\x38\x6E\x3F\x51\xE3\x0C"
			  "\x2E\x21\x58\xBC\x3E\x5F\xC7\x14\xC1\xEE\xEC\xA0\xEA\x69\x6D\x48",
	.iv		= "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	.plain	= "\x43\x10\x58\xF4\xDB\xC7\xF7\x34\xDA\x4F\x02\xF0\x4C\xC4\xF4\x59",
	.cipher	= "\x37\xFE\x26\xFF\x1C\xF6\x61\x75\xF5\xDD\xF4\xC3\x3B\x97\xA2\x05"
};

