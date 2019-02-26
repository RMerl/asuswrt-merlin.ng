/*
 * Copyright (C) 2016 Andreas Steffen
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
 * @defgroup newhope_noise newhope_noise
 * @{ @ingroup newhope_p
 */

#ifndef NEWHOPE_NOISE_H_
#define NEWHOPE_NOISE_H_

typedef struct newhope_noise_t newhope_noise_t;

#include <library.h>

/**
 * Generate pseudo random noise using a ChaCha20 stream
 * initialized with a 256 bit seed and an 8 bit nonce
 */
struct newhope_noise_t {

	/**
	 * Return n pseudo random bytes with a uniform distribution
	 *
	 * @param nonce		Nonce determining the pseudo random stream
	 * @param n			Number of pseudo random bytes to be returned
	 * @return			Return array with n peudo random bytes
	 */
	uint8_t* (*get_uniform_bytes)(newhope_noise_t *this, uint8_t nonce,
								  uint16_t n);

	/**
	 * Return n pseudo random 32-bit words with a Psi16 binomial distribution
	 *
	 * @param nonce		Nonce determining the pseudo random stream
	 * @param n			Number of pseudo random Psi16 words to be returned
	 * @param q			Prime number q determining the ring
	 * @return			Return array with n pseudo random 32 bit words
	 */
	uint32_t* (*get_binomial_words)(newhope_noise_t *this, uint8_t nonce,
									uint16_t n, uint16_t q);

	/**
	 * Destroy a newhope_noise_t object
	 */
	void (*destroy)(newhope_noise_t *this);
};

/**
 * Creates a new newhope_noise_t object.
 *
 * @param seed			256 bit seed (32 byte chunk)
 * @return				newhope_noise_t object, NULL if not supported
 */
newhope_noise_t *newhope_noise_create(chunk_t seed);

#endif /** NEWHOPE_NOISE_H_ @}*/

