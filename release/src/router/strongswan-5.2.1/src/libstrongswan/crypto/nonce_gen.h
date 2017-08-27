/*
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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
 * @defgroup nonce_gen nonce_gen
 * @{ @ingroup crypto
 */

#ifndef NONCE_GEN_H_
#define NONCE_GEN_H_

typedef struct nonce_gen_t nonce_gen_t;

#include <library.h>

/**
 * Generic interface for nonce generators.
 */
struct nonce_gen_t {

	/**
	 * Generates a nonce and writes it into the buffer.
	 *
	 * @param size		size of nonce in bytes
	 * @param buffer	pointer where the generated nonce will be written
	 * @return			TRUE if nonce allocation was successful, FALSE otherwise
	 */
	bool (*get_nonce)(nonce_gen_t *this, size_t size,
					  u_int8_t *buffer) __attribute__((warn_unused_result));

	/**
	 * Generates a nonce and allocates space for it.
	 *
	 * @param size		size of nonce in bytes
	 * @param chunk		chunk which will hold the generated nonce
	 * @return			TRUE if nonce allocation was successful, FALSE otherwise
	 */
	bool (*allocate_nonce)(nonce_gen_t *this, size_t size,
						   chunk_t *chunk) __attribute__((warn_unused_result));

	/**
	 * Destroys a nonce generator object.
	 */
	void (*destroy)(nonce_gen_t *this);
};

#endif /** NONCE_GEN_H_ @}*/
