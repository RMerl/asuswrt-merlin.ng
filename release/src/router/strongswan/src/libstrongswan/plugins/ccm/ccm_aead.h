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
 * @defgroup ccm_aead ccm_aead
 * @{ @ingroup ccm
 */

#ifndef CCM_AEAD_H_
#define CCM_AEAD_H_

#include <crypto/aead.h>

typedef struct ccm_aead_t ccm_aead_t;

/**
 * Counter with Cipher Block Chaining-Message Authentication Code (CCM).
 *
 * Implements CCM as specified in NIST 800-38B, using AEAD semantics from
 * RFC 5282, based on RFC4309.
 */
struct ccm_aead_t {

	/**
	 * Implements aead_t interface.
	 */
	aead_t aead;
};

/**
 * Create a ccm_aead instance.
 *
 * @param algo			algorithm to implement, a CCM mode
 * @param key_size		key size in bytes
 * @param salt_size		size of implicit salt length
 * @return				aead, NULL if not supported
 */
ccm_aead_t *ccm_aead_create(encryption_algorithm_t algo, size_t key_size,
							size_t salt_size);

#endif /** CCM_AEAD_H_ @}*/
