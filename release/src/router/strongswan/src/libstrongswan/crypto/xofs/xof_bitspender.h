/*
 * Copyright (C) 2014-2016 Andreas Steffen
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
 * @defgroup xof_bitspender xof_bitspender
 * @{ @ingroup mgf1
 */

#ifndef XOF_BITSPENDER_H_
#define XOF_BITSPENDER_H_

#include "xof.h"

#include <library.h>

typedef struct xof_bitspender_t xof_bitspender_t;

/**
 * Generates a given number of pseudo-random bits at a time using an
 * Extended Output Function (XOF)
 */
struct xof_bitspender_t {

	/**
	 * Get pseudo-random bits
	 *
	 * @param bits_needed	Number of needed bits (1..32)
	 * @param bits			Pseudo-random bits
	 * @result				FALSE if internal MGF1 error occurred
	 */
	bool (*get_bits)(xof_bitspender_t *this, int bits_needed, uint32_t *bits);

	/**
	 * Get a pseudo-random byte
	 *
	 * @param byte			Pseudo-random byte
	 * @result				FALSE if internal MGF1 error occurred
	 */
	bool (*get_byte)(xof_bitspender_t *this, uint8_t *byte);

	/**
	 * Destroy xof_bitspender_t object
	 */
	void (*destroy)(xof_bitspender_t *this);
};

/**
 * Create a xof_bitspender_t object
 *
 * @param alg				XOF to be used
 * @param seed				Seed used to initialize XOF
 * @param hash_seed			Hash seed before using it as a seed for MFG1
 */
xof_bitspender_t *xof_bitspender_create(ext_out_function_t alg, chunk_t seed,
										bool hash_seed);

#endif /** XOF_BITSPENDER_H_ @}*/
