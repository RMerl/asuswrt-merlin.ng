/*
 * Copyright (C) 2019 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup aesni_ecb aesni_ecb
 * @{ @ingroup aesni
 */

#ifndef AESNI_ECB_H_
#define AESNI_ECB_H_

#include <library.h>

typedef struct aesni_ecb_t aesni_ecb_t;

/**
 * ECB mode crypter using AES-NI
 */
struct aesni_ecb_t {

	/**
	 * Implements crypter interface
	 */
	crypter_t crypter;
};

/**
 * Create a aesni_ecb instance.
 *
 * @param algo			encryption algorithm, AES_ENCR_ECB
 * @param key_size		AES key size, in bytes
 * @return				AES-ECB crypter, NULL if not supported
 */
aesni_ecb_t *aesni_ecb_create(encryption_algorithm_t algo, size_t key_size);

#endif /** AESNI_ECB_H_ @}*/
