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
 * @defgroup bliss_bitpacker bliss_bitpacker
 * @{ @ingroup bliss_p
 */

#ifndef BLISS_BITPACKER_H_
#define BLISS_BITPACKER_H_

#include <library.h>

typedef struct bliss_bitpacker_t bliss_bitpacker_t;

/**
 * Reads and writes a variable number of bits in packed format
 * from and to an octet buffer
 */
struct bliss_bitpacker_t {

	/**
	 * Get the number of bits written into buffer
	 *
	 * @result			Number of bits written
	 */
	size_t (*get_bits)(bliss_bitpacker_t *this);

	/**
	 * Get the prime modulus of the Number Theoretic Transform
	 *
	 * @param value		Value to be written
	 * @param bits		Number of bits to be written
	 * @result			TRUE if value could be written into buffer
	 */
	bool (*write_bits)(bliss_bitpacker_t *this, uint32_t value, size_t bits);


	/**
	 * Get the prime modulus of the Number Theoretic Transform
	 *
	 * @param value		Value returned
	 * @param bits		Number of bits to be read
	 * @result			TRUE if value could be read from buffer
	 */
	bool (*read_bits)(bliss_bitpacker_t *this, uint32_t *value, size_t bits);

	/**
	 * Detach the internal octet buffer and return it
	 */
	chunk_t (*extract_buf)(bliss_bitpacker_t *this);

	/**
	 * Destroy bliss_bitpacker_t object
	 */
	void (*destroy)(bliss_bitpacker_t *this);
};

/**
 * Create a bliss_bitpacker_t object for writing
 *
 * @param max_bits		Total number of bits to be stored
 */
bliss_bitpacker_t* bliss_bitpacker_create(uint16_t max_bits);

/**
 * Create a bliss_bitpacker_t object for reading
 *
 * @param data			Packed array of bits
 */
bliss_bitpacker_t* bliss_bitpacker_create_from_data(chunk_t data);

#endif /** BLISS_BITPACKER_H_ @}*/
