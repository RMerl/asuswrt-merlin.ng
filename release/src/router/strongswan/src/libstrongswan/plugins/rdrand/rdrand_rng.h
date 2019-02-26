/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup rdrand_rng rdrand_rng
 * @{ @ingroup rdrand_p
 */

#ifndef RDRAND_RNG_H_
#define RDRAND_RNG_H_

#include <crypto/rngs/rng.h>

typedef struct rdrand_rng_t rdrand_rng_t;

/**
 * RNG implemented with Intels RDRAND instructions, introduced in Ivy Bridge.
 */
struct rdrand_rng_t {

	/**
	 * Implements rng_t interface.
	 */
	rng_t rng;
};

/**
 * Create a rdrand_rng instance.
 *
 * @param quality		RNG quality
 * @return				RNG instance
 */
rdrand_rng_t *rdrand_rng_create(rng_quality_t quality);

#endif /** RDRAND_RNG_H_ @}*/
