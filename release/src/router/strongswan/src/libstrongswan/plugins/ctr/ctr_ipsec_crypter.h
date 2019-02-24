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
 * @defgroup ctr_ipsec_crypter ctr_ipsec_crypter
 * @{ @ingroup ctr
 */

#ifndef CTR_IPSEC_CRYPTER_H_
#define CTR_IPSEC_CRYPTER_H_

#include <crypto/crypters/crypter.h>

typedef struct ctr_ipsec_crypter_t ctr_ipsec_crypter_t;

/**
 * Counter Mode wrapper for encryption algorithms, IPsec variant (RFC3686).
 */
struct ctr_ipsec_crypter_t {

	/**
	 * Implements crypter_t interface.
	 */
	crypter_t crypter;
};

/**
 * Create a ctr_ipsec_crypter instance.
 */
ctr_ipsec_crypter_t *ctr_ipsec_crypter_create();

/**
 * Create a ctr_ipsec_crypter instance.
 *
 * @param key_size		key size in bytes
 * @param algo			algorithm to implement, a counter mode
 * @return				crypter, NULL if not supported
 */
ctr_ipsec_crypter_t *ctr_ipsec_crypter_create(encryption_algorithm_t algo,
											  size_t key_size);

#endif /** CTR_IPSEC_CRYPTER_H_ @}*/
