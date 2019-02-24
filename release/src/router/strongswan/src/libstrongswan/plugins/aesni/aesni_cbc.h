/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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

/**
 * @defgroup aesni_cbc aesni_cbc
 * @{ @ingroup aesni
 */

#ifndef AESNI_CBC_H_
#define AESNI_CBC_H_

#include <library.h>

typedef struct aesni_cbc_t aesni_cbc_t;

/**
 * CBC mode crypter using AES-NI
 */
struct aesni_cbc_t {

	/**
	 * Implements crypter interface
	 */
	crypter_t crypter;
};

/**
 * Create a aesni_cbc instance.
 *
 * @param algo			encryption algorithm, AES_ENCR_CBC
 * @param key_size		AES key size, in bytes
 * @return				AES-CBC crypter, NULL if not supported
 */
aesni_cbc_t *aesni_cbc_create(encryption_algorithm_t algo, size_t key_size);

#endif /** AESNI_CBC_H_ @}*/
