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
 * @defgroup random_rng random_rng
 * @{ @ingroup random_p
 */

#ifndef RANDOM_RNG_H_
#define RANDOM_RNG_H_

typedef struct random_rng_t random_rng_t;

#include <library.h>

/**
 * rng_t implementation on top of /dev/[u]random
 */
struct random_rng_t {

	/**
	 * Implements rng_t.
	 */
	rng_t rng;
};

/**
 * Creates an random_rng_t instance.
 *
 * @param quality	required quality of randomness
 * @return			created random_rng_t
 */
random_rng_t *random_rng_create(rng_quality_t quality);

#endif /** RANDOM_RNG_H_ @} */
