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
 * @defgroup bliss_huffman_coder bliss_huffman_coder
 * @{ @ingroup bliss_p
 */

#ifndef BLISS_HUFFMAN_CODER_H_
#define BLISS_HUFFMAN_CODER_H_

#include "bliss_huffman_code.h"
#include "bliss_bitpacker.h"

#include <library.h>

typedef struct bliss_huffman_coder_t bliss_huffman_coder_t;

/**
 * Encodes and decodes binary Huffman codes
 */
struct bliss_huffman_coder_t {

	/**
	 * Get number of encoded or decoded bits
	 *
	 * @result			Number of bits
	 */
	size_t (*get_bits)(bliss_huffman_coder_t *this);

	/**
	 * Encode a (z1, z2) tuple using a Huffman code
	 *
	 * @param z1		z1 value to be encoded
	 * @param z2		z2 value to be encoded
	 * @result			TRUE if value could be encoded
	 */
	bool (*encode)(bliss_huffman_coder_t *this, int32_t z1, int16_t z2);


	/**
	 * Decode a (z1, z2) tuple using a Huffman code
	 *
	 * @param z1		Decoded z1 value returned
	 * @param z2		Decoded z2 value returned
	 * @result			TRUE if value could be decoded from bitpacker
	 */
	bool (*decode)(bliss_huffman_coder_t *this, int32_t *z1, int16_t *z2);

	/**
	 * Destroy bliss_huffman_coder_t object
	 */
	void (*destroy)(bliss_huffman_coder_t *this);
};

/**
 * Create a bliss_huffman_coder_t object
 *
 * @param code			Huffman code table
 * @param packer		Bitpacker to write to or read from
 */
bliss_huffman_coder_t* bliss_huffman_coder_create(bliss_huffman_code_t *code,
												  bliss_bitpacker_t *packer);

#endif /** BLISS_HUFFMAN_CODER_H_ @}*/
