/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2009 Andreas Steffen
 * Copyright (C) JuanJo Ciarlante <jjo-ipsec@mendoza.gov.ar>
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
 * Test vector by Eric Young
 */
crypter_test_vector_t blowfish1 = {
	.alg = ENCR_BLOWFISH, .key_size = 16, .len = 32,
	.key	= "\x01\x23\x45\x67\x89\xAB\xCD\xEF\xF0\xE1\xD2\xC3\xB4\xA5\x96\x87",
	.iv		= "\xFE\xDC\xBA\x98\x76\x54\x32\x10",
	.plain	= "7654321 Now is the time for \0\0\0\0",
	.cipher	= "\x6B\x77\xB4\xD6\x30\x06\xDE\xE6\x05\xB1\x56\xE2\x74\x03\x97\x93"
			  "\x58\xDE\xB9\xE7\x15\x46\x16\xD9\x59\xF1\x65\x2B\xD5\xFF\x92\xCC"
};

/**
 * Test vector by Chilkat Software
 * (www.chilkatsoft.com/p/php_blowfish.asp)
 */
crypter_test_vector_t blowfish2 = {
	.alg = ENCR_BLOWFISH, .key_size = 32, .len = 48,
	.key	= "\x31\x32\x33\x34\x35\x36\x37\x38\x39\x30\x31\x32\x33\x34\x35\x36"
			  "\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50",
	.iv		= "\x31\x32\x33\x34\x35\x36\x37\x38",
	.plain	= "The quick brown fox jumped over the lazy dog\0\0\0\0",
	.cipher	= "\x27\x68\x55\xca\x6c\x0d\x60\xf7\xd9\x70\x82\x10\x44\x0c\x10\x72"
			  "\xe0\x5d\x07\x8e\x73\x3b\x34\xb4\x19\x8d\x60\x9d\xc2\xfc\xc2\xf0"
			  "\xc3\x09\x26\xcd\xef\x3b\x6d\x52\xba\xf6\xe3\x45\xaa\x03\xf8\x3e"
};

