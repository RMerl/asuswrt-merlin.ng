/*
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
 * @defgroup padlock_rng padlock_rng
 * @{ @ingroup padlock_p
 */

#ifndef PADLOCK_RNG_H_
#define PADLOCK_RNG_H_

#include <crypto/rngs/rng.h>

typedef struct padlock_rng_t padlock_rng_t;

/**
 * Hardware-RNG based on via Padlock.
 */
struct padlock_rng_t {

	/**
	 * Implements rng_t interface.
	 */
	rng_t rng;
};

/**
 * Create a padlock_rng instance.
 *
 * @param quality	required quality of randomness
 * @return			created random_rng_t
 */
padlock_rng_t *padlock_rng_create(rng_quality_t quality);

#endif /** PADLOCK_RNG_H_ @}*/
