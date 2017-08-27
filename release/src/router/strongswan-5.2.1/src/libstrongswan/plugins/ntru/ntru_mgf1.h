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
 * @defgroup ntru_mgf1 ntru_mgf1
 * @{ @ingroup ntru_p
 */

#ifndef NTRU_MGF1_H_
#define NTRU_MGF1_H_

typedef struct ntru_mgf1_t ntru_mgf1_t;

#include <library.h>

/**
 * Implements the PKCS#1 MGF1 Mask Generation Function based on a hash function
 * defined in section 10.2.1 of RFC 2437 
 */
struct ntru_mgf1_t {

	/**
	 * Get the hash size of the underlying hash function
	 *
	 * @return			hash size in bytes
	 */
	size_t (*get_hash_size)(ntru_mgf1_t *this);

	/**
	 * Generate a mask pattern and copy it to an output buffer
	 * If the maximum number of requests has been reached, reseeding occurs
	 *
	 * @param mask_len	number of mask bytes to generate
	 * @param mask		output buffer of minimum size mask_len
	 * @return			TRUE if successful
	 */
	bool (*get_mask)(ntru_mgf1_t *this, size_t mask_len, u_char *mask);

	/**
	 * Generate a mask pattern and return it in an allocated chunk
	 *
	 * @param mask_len	number of mask bytes to generate
	 * @param mask		chunk containing generated mask
	 * @return			TRUE if successful
	 */
	bool (*allocate_mask)(ntru_mgf1_t *this, size_t mask_len, chunk_t *mask);

	/**
	 * Destroy the MGF1 object
	 */
	void (*destroy)(ntru_mgf1_t *this);
};

/**
 * Create an MGF1 object
 *
 * @param alg			hash algorithm to be used by MGF1
 * @param seed			seed used by MGF1 to generate mask from
 * @param hash_seed		hash seed before using it as a seed from MGF1
 */
ntru_mgf1_t *ntru_mgf1_create(hash_algorithm_t alg, chunk_t seed,
							  bool hash_seed);

#endif /** NTRU_MGF1_H_ @}*/

