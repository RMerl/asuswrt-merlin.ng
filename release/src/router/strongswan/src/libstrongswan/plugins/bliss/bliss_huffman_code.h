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
 * @defgroup bliss_huffman_code bliss_huffman_code
 * @{ @ingroup bliss_p
 */

#ifndef BLISS_HUFFMAN_CODE_H_
#define BLISS_HUFFMAN_CODE_H_

#include "bliss_param_set.h"

#include <library.h>

typedef struct bliss_huffman_code_t bliss_huffman_code_t;
typedef struct bliss_huffman_code_tuple_t bliss_huffman_code_tuple_t;
typedef struct bliss_huffman_code_node_t bliss_huffman_code_node_t;

struct bliss_huffman_code_tuple_t {
	uint32_t code;
	uint16_t bits;
};

#define BLISS_HUFFMAN_CODE_NO_TUPLE		-1
#define BLISS_HUFFMAN_CODE_NO_NODE		-1

struct bliss_huffman_code_node_t {
	int16_t node_0;
	int16_t node_1;
	int16_t tuple;
};

/**
 * Defines the Huffman code for the optimum encoding of a BLISS signature
 */
struct bliss_huffman_code_t {

	/**
	 * Range of z1:  0..n_z1-1
	 */
	uint16_t n_z1;

	/**
	 * Range of z2:  -n_z2..n_z2
	 */
	uint16_t n_z2;

	/**
	 * Table of tuple codewords
	 */
	bliss_huffman_code_tuple_t *tuples;

	/**
	 * Table of binary decision nodes
	 */
	bliss_huffman_code_node_t *nodes;
};

/**
 * Get Optimum Huffman code for BLISS signature given by BLISS parameter set ID
 *
 * @param id	BLISS parameter set ID
 * @return		Optimum Huffman code for BLISS signature
*/
bliss_huffman_code_t* bliss_huffman_code_get_by_id(bliss_param_set_id_t id);

#endif /** BLISS_HUFFMAN_CODE_H_ @}*/
