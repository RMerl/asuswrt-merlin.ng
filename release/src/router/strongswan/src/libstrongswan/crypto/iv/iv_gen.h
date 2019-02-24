/*
 * Copyright (C) 2013 Tobias Brunner
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
 * @defgroup iv iv
 * @{ @ingroup crypto
 */

#ifndef IV_GEN_H_
#define IV_GEN_H_

typedef struct iv_gen_t iv_gen_t;

#include <library.h>

/**
 * Generic interface for initialization vector (IV) generators.
 */
struct iv_gen_t {

	/**
	 * Generates an IV and writes it into the buffer.
	 *
	 * @param seq		external sequence number
	 * @param size		size of IV in bytes
	 * @param buffer	pointer where the generated IV will be written
	 * @return			TRUE if IV allocation was successful, FALSE otherwise
	 */
	bool (*get_iv)(iv_gen_t *this, uint64_t seq, size_t size,
				   uint8_t *buffer) __attribute__((warn_unused_result));

	/**
	 * Generates an IV and allocates space for it.
	 *
	 * @param seq		external sequence number
	 * @param size		size of IV in bytes
	 * @param chunk		chunk which will hold the generated IV
	 * @return			TRUE if IV allocation was successful, FALSE otherwise
	 */
	bool (*allocate_iv)(iv_gen_t *this, uint64_t seq, size_t size,
						chunk_t *chunk) __attribute__((warn_unused_result));

	/**
	 * Destroys an IV generator object.
	 */
	void (*destroy)(iv_gen_t *this);
};

/**
 * Select an IV generator for a given encryption algorithm.
 *
 * @param alg			encryption algorithm
 * @return				IV generator
 */
iv_gen_t* iv_gen_create_for_alg(encryption_algorithm_t alg);

#endif /** IV_GEN_H_ @}*/
