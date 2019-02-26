/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup gcm_aead gcm_aead
 * @{ @ingroup gcm
 */

#ifndef GCM_AEAD_H_
#define GCM_AEAD_H_

#include <crypto/aead.h>

typedef struct gcm_aead_t gcm_aead_t;

/**
 * Galois/Counter Mode (GCM).
 *
 * Implements GCM as specified in NIST 800-38D, using AEAD semantics from
 * RFC 5282, based on RFC4106.
 */
struct gcm_aead_t {

	/**
	 * Implements aead_t interface.
	 */
	aead_t aead;
};

/**
 * Create a gcm_aead instance.
 *
 * @param algo			algorithm to implement, a gcm mode
 * @param key_size		key size in bytes
 * @param salt_size		size of implicit salt length
 * @return				aead, NULL if not supported
 */
gcm_aead_t *gcm_aead_create(encryption_algorithm_t algo, size_t key_size,
							size_t salt_size);

#endif /** GCM_AEAD_H_ @}*/
