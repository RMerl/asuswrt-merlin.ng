/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup ntru_drbg ntru_drbg
 * @{ @ingroup ntru_p
 */

#ifndef NTRU_DRBG_H_
#define NTRU_DRBG_H_

typedef struct ntru_drbg_t ntru_drbg_t;

#include <library.h>

/**
 * Implements a HMAC Deterministic Random Bit Generator (HMAC_DRBG)
 * compliant with NIST SP 800-90A
 */
struct ntru_drbg_t {

	/**
	 * Reseed the instantiated DRBG
	 *
	 * @return			configured security strength in bits
	 */
	u_int32_t (*get_strength)(ntru_drbg_t *this);

	/**
	 * Reseed the instantiated DRBG
	 *
	 * @return			TRUE if successful
	 */
	bool (*reseed)(ntru_drbg_t *this);

	/**
	 * Generate pseudorandom bytes.
	 * If the maximum number of requests has been reached, reseeding occurs
	 *
	 * @param strength	requested security strength in bits
	 * @param len		number of octets to generate
	 * @param out		address of output buffer
	 * @return			TRUE if successful
	 */
	bool (*generate)(ntru_drbg_t *this, u_int32_t strength, u_int32_t len,
										u_int8_t *out);

	/**
	 * Get a reference on an ntru_drbg_t object increasing the count by one
	 *
	 * @return			reference to the ntru_drbg_t object
	 */
	ntru_drbg_t* (*get_ref)(ntru_drbg_t *this);

	/**
	 * Uninstantiate and destroy the DRBG object
	 */
	void (*destroy)(ntru_drbg_t *this);
};

/**
 * Create and instantiate a new DRBG objet.
 *
 * @param strength		security strength in bits
 * @param pers_str		personalization string
 * @param entropy		entropy source to use
 */
ntru_drbg_t *ntru_drbg_create(u_int32_t strength, chunk_t pers_str,
							  rng_t *entropy);

#endif /** NTRU_DRBG_H_ @}*/

