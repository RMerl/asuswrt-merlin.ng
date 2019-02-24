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
 * @defgroup aesni_ctr aesni_ctr
 * @{ @ingroup aesni
 */

#ifndef AESNI_CTR_H_
#define AESNI_CTR_H_

#include <library.h>

typedef struct aesni_ctr_t aesni_ctr_t;

/**
 * CTR mode crypter using AES-NI
 */
struct aesni_ctr_t {

	/**
	 * Implements crypter interface
	 */
	crypter_t crypter;
};

/**
 * Create a aesni_ctr instance.
 *
 * @param algo			encryption algorithm, AES_ENCR_CTR
 * @param key_size		AES key size, in bytes
 * @return				AES-CTR crypter, NULL if not supported
 */
aesni_ctr_t *aesni_ctr_create(encryption_algorithm_t algo, size_t key_size);

#endif /** AESNI_CTR_H_ @}*/
