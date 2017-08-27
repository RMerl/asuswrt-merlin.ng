/*
 * Copyright (C) 2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup gcrypt_rng gcrypt_rng
 * @{ @ingroup gcrypt_p
 */

#ifndef GCRYPT_RNG_H_
#define GCRYPT_RNG_H_

typedef struct gcrypt_rng_t gcrypt_rng_t;

#include <library.h>

/**
 * rng_t implementation using libgcrypt.
 */
struct gcrypt_rng_t {

	/**
	 * Implements rng_t.
	 */
	rng_t rng;
};

/**
 * Creates an gcrypt_rng_t instance.
 *
 * @param quality	required quality of gcryptness
 * @return			created gcrypt_rng_t
 */
gcrypt_rng_t *gcrypt_rng_create(rng_quality_t quality);

#endif /** GCRYPT_RNG_H_ @} */
