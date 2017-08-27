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
 * @defgroup ntru_trits ntru_trits
 * @{ @ingroup ntru_p
 */

#ifndef NTRU_TRITS_H_
#define NTRU_TRITS_H_

typedef struct ntru_trits_t ntru_trits_t;

#include <library.h>

/**
 * Implements an array of trinary elements (trits) 
 */
struct ntru_trits_t {

	/**
	 * Get the size of the trits array
	 *
	 * @return			number of trinary elements
	 */
	size_t (*get_size)(ntru_trits_t *this);

	/**
	 * @return			octet array containing a trit per octet
	 */
	uint8_t* (*get_trits)(ntru_trits_t *this);

	/**
	 * Destroy ntru_trits_t object
	 */
	void (*destroy)(ntru_trits_t *this);
};

/**
 * Create a trits array from a seed using MGF1 with a base hash function
 *
 * @param size			size of the trits array
 * @param alg			hash algorithm to be used by MGF1
 * @param seed			seed used by MGF1 to generate trits from
 */
ntru_trits_t *ntru_trits_create(size_t size, hash_algorithm_t alg, chunk_t seed);

#endif /** NTRU_TRITS_H_ @}*/

