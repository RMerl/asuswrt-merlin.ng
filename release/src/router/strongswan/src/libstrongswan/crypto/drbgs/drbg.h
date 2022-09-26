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
 * @defgroup drbg drbg
 * @{ @ingroup crypto
 */

#ifndef DRBG_H_
#define DRBG_H_

typedef enum drbg_type_t drbg_type_t;
typedef struct drbg_t drbg_t;

#include <library.h>

/**
 * Deterministic Random Bit Generator.(DRBG) Type
 */
enum drbg_type_t {
	DRBG_UNDEFINED,
	DRBG_HMAC_SHA1,
	DRBG_HMAC_SHA256,
	DRBG_HMAC_SHA384,
	DRBG_HMAC_SHA512,
	DRBG_CTR_AES128,
	DRBG_CTR_AES192,
	DRBG_CTR_AES256,
};

/**
 * enum name for drbg_type_t.
 */
extern enum_name_t *drbg_type_names;

/**
 * Interface for a Deterministic Random Bit Generator (DRBG)
 * compliant with NIST SP 800-90A
 */
struct drbg_t {

	/**
	 * Return the type of the DRBG
	 *
	 * @return			DRBG type
	 */
	drbg_type_t (*get_type)(drbg_t *this);

	/**
	 * Return the security strength of the  DRBG
	 *
	 * @return			configured security strength in bits
	 */
	uint32_t (*get_strength)(drbg_t *this);

	/**
	 * Reseed the instantiated DRBG
	 *
	 * @return			TRUE if successful
	 */
	bool (*reseed)(drbg_t *this);

	/**
	 * Generate pseudorandom bytes.
	 * If the maximum number of requests has been reached, reseeding occurs
	 *
	 * @param len		number of octets to generate
	 * @param out		address of output buffer
	 * @return			TRUE if successful
	 */
	bool (*generate)(drbg_t *this, uint32_t len, uint8_t *out);

	/**
	 * Get a reference on an drbg_t object increasing the count by one
	 *
	 * @return			reference to the drbg_t object
	 */
	drbg_t* (*get_ref)(drbg_t *this);

	/**
	 * Uninstantiate and destroy the DRBG object
	 */
	void (*destroy)(drbg_t *this);

};

#endif /** DRBG_H_ @}*/
