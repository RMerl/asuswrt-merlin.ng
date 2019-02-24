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

#include "bliss_signature.h"
#include "bliss_bitpacker.h"
#include "bliss_huffman_coder.h"


typedef struct private_bliss_signature_t private_bliss_signature_t;

/**
 * Private data of a bliss_signature_t object.
 */
struct private_bliss_signature_t {
	/**
	 * Public interface for this signer.
	 */
	bliss_signature_t public;

	/**
	 * BLISS signature parameter set
	 */
	const bliss_param_set_t *set;

	/**
	 * BLISS signature vector z1 of size n
	 */
	int32_t *z1;

	/**
	 * BLISS signature vector z2d of size n
	 */
	int16_t *z2d;

	/**
	 * Indices of sparse BLISS challenge vector c of size kappa
	 */
	uint16_t *c_indices;

};

METHOD(bliss_signature_t, get_encoding, chunk_t,
	private_bliss_signature_t *this)
{
	bliss_bitpacker_t *packer;
	bliss_huffman_coder_t *coder;
	bliss_huffman_code_t *code;
	int32_t z1;
	uint32_t z1_sign;
	uint16_t z2d_bits;
	chunk_t encoding = chunk_empty;
	int i;

	z2d_bits = this->set->z1_bits - this->set->d;

	/* Get Huffman code for this BLISS parameter set */
	code = bliss_huffman_code_get_by_id(this->set->id);
	if (!code)
	{
		DBG1(DBG_LIB, "no Huffman code found for parameter set %N",
			 bliss_param_set_id_names, this->set->id);
		return chunk_empty;
	}

	packer = bliss_bitpacker_create(this->set->n * this->set->z1_bits +
									this->set->n * z2d_bits +
									this->set->kappa * this->set->n_bits);
	coder = bliss_huffman_coder_create(code, packer);

	for (i = 0; i < this->set->n; i++)
	{
		/* determine and remove the sign of z1[i]*/
		z1_sign = this->z1[i] < 0;
		z1 = z1_sign ? -this->z1[i] : this->z1[i];

		if (!packer->write_bits(packer, z1_sign, 1) ||
			!packer->write_bits(packer, z1 & 0xff, 8) ||
			!coder->encode(coder, z1 >> 8, this->z2d[i]))
		{
			goto end;
		}
	}
	for (i = 0; i < this->set->kappa; i++)
	{
		if (!packer->write_bits(packer, this->c_indices[i], this->set->n_bits))
		{
			goto end;
		}
	}
	encoding = packer->extract_buf(packer);

	DBG2(DBG_LIB, "efficiency of Huffman coder is %6.4f bits/tuple (%u bits)",
				   coder->get_bits(coder)/(double)(this->set->n), 
				   coder->get_bits(coder));
	DBG2(DBG_LIB, "generated BLISS signature (%u bits encoded in %u bytes)",
				   packer->get_bits(packer), encoding.len);

	end:
	coder->destroy(coder);
	packer->destroy(packer);
	return encoding;
}

METHOD(bliss_signature_t, get_parameters, void,
	private_bliss_signature_t *this, int32_t **z1, int16_t **z2d,
	uint16_t **c_indices)
{
	*z1 = this->z1;
	*z2d = this->z2d;
	*c_indices = this->c_indices;
}

METHOD(bliss_signature_t, destroy, void,
	private_bliss_signature_t *this)
{
	free(this->z1);
	free(this->z2d);
	free(this->c_indices);
	free(this);
}

/**
 * See header.
 */
bliss_signature_t *bliss_signature_create(const bliss_param_set_t *set)
{
	private_bliss_signature_t *this;

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.get_parameters = _get_parameters,
			.destroy = _destroy,
		},
		.set = set,
		.z1  = malloc(set->n * sizeof(int32_t)),
		.z2d = malloc(set->n * sizeof(int16_t)),
		.c_indices = malloc(set->n * sizeof(uint16_t)),
	);

	return &this->public;
}

/**
 * See header.
 */
bliss_signature_t *bliss_signature_create_from_data(const bliss_param_set_t *set,
													chunk_t encoding)
{
	private_bliss_signature_t *this;
	bliss_bitpacker_t *packer;
	bliss_huffman_coder_t *coder;
	bliss_huffman_code_t *code;
	uint32_t z1_sign, z1_low, value;
	int32_t z1;
	int16_t z2;
	int i;

	/* Get Huffman code for this BLISS parameter set */
	code = bliss_huffman_code_get_by_id(set->id);
	if (!code)
	{
		DBG1(DBG_LIB, "no Huffman code found for parameter set %N",
			 bliss_param_set_id_names, set->id);
		return NULL;
	}

	if (encoding.len == 0)
	{
		DBG1(DBG_LIB, "zero length BLISS signature");
		return NULL;
	}

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.get_parameters = _get_parameters,
			.destroy = _destroy,
		},
		.set = set,
		.z1  = malloc(set->n * sizeof(int32_t)),
		.z2d = malloc(set->n * sizeof(int16_t)),
		.c_indices = malloc(set->n * sizeof(uint16_t)),
	);

	packer = bliss_bitpacker_create_from_data(encoding);
	coder = bliss_huffman_coder_create(code, packer);

	for (i = 0; i < set->n; i++)
	{
		if (!packer->read_bits(packer, &z1_sign, 1) ||
			!packer->read_bits(packer, &z1_low, 8) ||
			!coder->decode(coder, &z1, &z2))
		{
			DBG1(DBG_LIB, "truncated BLISS signature encoding of z1/z2");
			coder->destroy(coder);
			packer->destroy(packer);
			destroy(this);
			return NULL;
		}
		z1 = (z1 << 8) + z1_low;
		this->z1[i] = z1_sign ? -z1 : z1;
		this->z2d[i] = z2;
	}
	coder->destroy(coder);

	for (i = 0; i < set->kappa; i++)
	{
		if (!packer->read_bits(packer, &value, set->n_bits))
		{
			DBG1(DBG_LIB, "truncated BLISS signature encoding of c_indices");
			packer->destroy(packer);
			destroy(this);
			return NULL;
		}
		this->c_indices[i] = value;
	}
	packer->destroy(packer);

	return &this->public;
}
