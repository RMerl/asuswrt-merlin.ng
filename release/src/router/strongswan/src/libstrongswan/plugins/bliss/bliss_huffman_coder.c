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
 * WITHOUT ANY WARRANTY;https://www.hsr.ch/HSR-intern-Anmeldung.4409.0.html?&no_cache=1 without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "bliss_huffman_coder.h"

typedef struct private_bliss_huffman_coder_t private_bliss_huffman_coder_t;

/**
 * Private data structure for bliss_huffman_coder_t object
 */
struct private_bliss_huffman_coder_t {
	/**
	 * Public interface.
	 */
	bliss_huffman_coder_t public;

	/**
	 * Bitpacker to write to or read from
	 */
	bliss_bitpacker_t *packer;

	/**
	 * Huffman code table to be used
	 */
	bliss_huffman_code_t *code;

	/**
     * Maximum index into tuples table
	 */
	int index_max;

	/**
     * Number of encoded or decoded bits
	 */
	size_t bits;

};

METHOD(bliss_huffman_coder_t, get_bits, size_t,
	private_bliss_huffman_coder_t *this)
{
	return this->bits;
}

METHOD(bliss_huffman_coder_t, encode, bool,
	private_bliss_huffman_coder_t *this, int32_t z1, int16_t z2)
{
	uint32_t code;
	uint16_t bits;
	int index;

	index = z1 * (2*this->code->n_z2 - 1) + z2 + this->code->n_z2 - 1;
	if (index >= this->index_max)
	{
		DBG1(DBG_LIB, "index exceeded in Huffman encoding table");
		return FALSE;
	}
	code = this->code->tuples[index].code;
	bits = this->code->tuples[index].bits;
	if (!this->packer->write_bits(this->packer, code, bits))
	{
		DBG1(DBG_LIB, "bitpacker exceeded its buffer");
		return FALSE;
	}
	this->bits += bits;

	return TRUE;
}

METHOD(bliss_huffman_coder_t, decode, bool,
	private_bliss_huffman_coder_t *this, int32_t *z1, int16_t *z2)
{
	bliss_huffman_code_node_t *node;
	uint32_t bit;

	node = this->code->nodes;
	while (node->tuple == BLISS_HUFFMAN_CODE_NO_TUPLE)
	{
		if (node->node_0 == BLISS_HUFFMAN_CODE_NO_NODE ||
			node->node_1 == BLISS_HUFFMAN_CODE_NO_NODE)
		{
			DBG1(DBG_LIB, "error in Huffman decoding table");
			return FALSE;
		}
		if (!this->packer->read_bits(this->packer, &bit, 1))
		{
			DBG1(DBG_LIB, "bitpacker depleted its buffer");
			return FALSE;
		}
		node = &this->code->nodes[bit ? node->node_1 : node->node_0];
		this->bits++;
	}
	*z1 = node->tuple / (2*this->code->n_z2 - 1);
	*z2 = node->tuple - (2*this->code->n_z2 - 1) * (*z1) - this->code->n_z2 + 1;

	return TRUE;
}

METHOD(bliss_huffman_coder_t, destroy, void,
	private_bliss_huffman_coder_t *this)
{
	free(this);
}

/**
 * See header.
 */
bliss_huffman_coder_t *bliss_huffman_coder_create(bliss_huffman_code_t *code,
												  bliss_bitpacker_t *packer)
{
	private_bliss_huffman_coder_t *this;

	INIT(this,
		.public = {
			.get_bits = _get_bits,
			.encode = _encode,
			.decode = _decode,
			.destroy = _destroy,
		},
		.packer = packer,
		.code = code,
		.index_max = (2*code->n_z2 - 1) * code->n_z1,		
	);

	return &this->public;
}
