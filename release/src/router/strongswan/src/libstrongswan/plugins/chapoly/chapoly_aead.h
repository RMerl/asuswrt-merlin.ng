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
 * @defgroup chapoly_aead chapoly_aead
 * @{ @ingroup chapoly
 */

#ifndef CHAPOLY_AEAD_H_
#define CHAPOLY_AEAD_H_

#include <crypto/aead.h>

typedef struct chapoly_aead_t chapoly_aead_t;

/**
 * ChaCha20/Poly1305 AEAD implementation.
 *
 * TODO-Chapoly: draft-ietf-ipsecme-chacha20-poly1305-05
 */
struct chapoly_aead_t {

	/**
	 * Implements aead_t interface.
	 */
	aead_t aead;
};

/**
 * Create a chapoly_aead instance.
 *
 * @param algo			algorithm to implement, ENCR_CHACHA20_POLY1305
 * @param key_size		key size in bytes, 32
 * @param salt_size		size of implicit salt length, 0
 * @return				AEAD, NULL if not supported
 */
chapoly_aead_t *chapoly_aead_create(encryption_algorithm_t algo,
									size_t key_size, size_t salt_size);

#endif /** CHAPOLY_AEAD_H_ @}*/
