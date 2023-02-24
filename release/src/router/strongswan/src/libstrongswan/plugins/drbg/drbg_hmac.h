/*
 * Copyright (C) 2019 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup drbg_hmac drbg_hmac
 * @{ @ingroup drbg_p
 */

#ifndef DRBG_HMAC_H_
#define DRBG_HMAC_H_

#include <crypto/drbgs/drbg.h>

typedef struct drbg_hmac_t drbg_hmac_t;

/**
 * NIST SP 800-90A HMAC DRBC implementation
 */
struct drbg_hmac_t {

	/**
	 * Public Deterministic Random Bit Generator (DRBG) Interface
	 */
	drbg_t interface;
};

/**
 * Create a drbg_hmac instance.
 *
 * @param type					DRBG HMAC type
 * @param strength				security strength in bits
 * @param entropy				entropy source to be used
 * @param personalization_str	optional personalization string
 * @return						drbg_hmac_t object, NULL if not supported
 */
drbg_hmac_t *drbg_hmac_create(drbg_type_t type, uint32_t strength,
							  rng_t *entropy, chunk_t personalization_str);

#endif /** DRBG_HMAC_H_ @}*/
