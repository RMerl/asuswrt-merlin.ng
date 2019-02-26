/*
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup bliss_sampler bliss_sampler
 * @{ @ingroup bliss_p
 */

#ifndef BLISS_SAMPLER_H_
#define BLISS_SAMPLER_H_

typedef struct bliss_sampler_t bliss_sampler_t;

#include "bliss_param_set.h"

#include <library.h>
#include <crypto/hashers/hasher.h>

/**
 * Implementation various rejection sampling algorithms.
 */
struct bliss_sampler_t {

	/**
	 * Sample according to exp(-x/(2*sigma^2))
	 *
	 * @param x			Value to be sampled
	 * @param accepted	TRUE if value is accepted, FALSE if rejected
	 * @result			TRUE if sampling was successful
	 */
	bool (*bernoulli_exp)(bliss_sampler_t *this, uint32_t x, bool *accepted);

	/**
	 * Sample according to 1/cosh(x/sigma^2)
	 *
	 * @param x			Value to be sampled
	 * @param accepted	TRUE if value is accepted, FALSE if rejected
	 * @result			TRUE if sampling was successful
	 */
	bool (*bernoulli_cosh)(bliss_sampler_t *this, int32_t x, bool *accepted);

	/**
	 * Sample according to 2^(-x^2) for positive x
	 *
	 * @param x			Generated value
	 * @result			TRUE if sampling was successful
	 */
	bool (*pos_binary)(bliss_sampler_t *this, uint32_t *x);

	/**
	 * Sample according to the Gaussian distribution exp(-x^2/(2*sigma^2))
	 *
	 * @param z			Generated value with Gaussian distribution
	 * @result			TRUE if sampling was successful
	 */
	bool (*gaussian)(bliss_sampler_t *this, int32_t *z);

	/**
	 * Sample the sign according to the binary distribution
	 *
	 * @param positive	TRUE if positive
	 * @result			TRUE if sampling was successful
	 */
	bool (*sign)(bliss_sampler_t *this, bool *positive);

	/**
	 * Destroy bliss_sampler_t object
	 */
	void (*destroy)(bliss_sampler_t *this);
};

/**
 * Create a bliss_sampler_t object.
 *
 * @param alg		XOF to be used for the internal bitspender
 * @param seed		Seed used to initialize the internal bitspender
 * @param set		BLISS parameter set to be used
 */
bliss_sampler_t *bliss_sampler_create(ext_out_function_t alg, chunk_t seed,
									  const bliss_param_set_t *set);

#endif /** BLISS_SAMPLER_H_ @}*/
