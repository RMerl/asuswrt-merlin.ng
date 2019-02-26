/*
 * Copyright (C) 2008 Thomas Kallenberg
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup padlock_aes_crypter padlock_aes_crypter
 * @{ @ingroup padlock_p
 */

#ifndef PADLOCK_AES_CRYPTER_H_
#define PADLOCK_AES_CRYPTER_H_

typedef struct padlock_aes_crypter_t padlock_aes_crypter_t;

#include <crypto/crypters/crypter.h>

/**
 * Implementation of AES-128 using VIA Padlock.
 */
struct padlock_aes_crypter_t {

	/**
	 * Implements crypter_t interface.
	 */
	crypter_t crypter;
};

/**
 * Constructor to create padlock_aes_crypter_t.
 *
 * @param key_size		key size in bytes, currently supports only 16.
 * @param algo			algorithm to implement, must be ENCR_AES_CBC
 * @return				padlock_aes_crypter_t, NULL if not supported
 */
padlock_aes_crypter_t *padlock_aes_crypter_create(encryption_algorithm_t algo,
												  size_t key_size);

#endif /** PADLOCK_AES_CRYPTER_H_ @}*/
