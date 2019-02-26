/*
 * Copyright (C) 2014-2016 Andreas Steffen
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

#include "xof_bitspender.h"
#include "mgf1.h"

typedef struct private_xof_bitspender_t private_xof_bitspender_t;

/**
 * Private data structure for xof_bitspender_t object
 */
struct private_xof_bitspender_t {
	/**
	 * Public interface.
	 */
	xof_bitspender_t public;

	/**
	 * Extended Output Function (XOF)
	 */
	xof_t *xof;

	/**
	 * Length of the returned hash value in octets
	 */
	int hash_len;

	/**
	 * Bit storage (accommodates up to 32 bits)
	 */
	uint32_t bits;

	/**
	 * Number of available bits
	 */
	int bits_left;

	/**
	 * Byte storage (accommodates up to 4 bytes)
	 */
	uint8_t bytes[4];

	/**
	 * Number of available bytes
	 */
	int bytes_left;

	/**
	 * Number of octets spent
	 */
	int octet_count;

};

static bool get_next_block(private_xof_bitspender_t *this, uint8_t *buffer)
{
	if (!this->xof->get_bytes(this->xof, 4, buffer))
	{
		/* no block available */
		return FALSE;
	}
	this->octet_count += 4;

	return TRUE;
}

METHOD(xof_bitspender_t, get_bits, bool,
	private_xof_bitspender_t *this, int bits_needed, uint32_t *bits)
{
	int bits_now;

	*bits = 0x00000000;

	if (bits_needed == 0)
	{
		/* trivial */
		return TRUE;
	}
	if (bits_needed > 32)
	{
		/* too many bits requested */
		return FALSE;
	}

	while (bits_needed)
	{
		if (this->bits_left == 0)
		{
			uint8_t buf[4];

			if (!get_next_block(this, buf))
			{
				return FALSE;
			}
			this->bits = untoh32(buf);
			this->bits_left = 32;
		}
		if (bits_needed > this->bits_left)
		{
			bits_now = this->bits_left;
			this->bits_left = 0;
			bits_needed -= bits_now;
		}
		else
		{
			bits_now = bits_needed;
			this->bits_left -= bits_needed;
			bits_needed = 0;
		}
		if (bits_now == 32)
		{
			*bits = this->bits;
		}
		else
		{
			*bits <<= bits_now;
			*bits |= this->bits >> this->bits_left;
			if (this->bits_left)
			{
				this->bits &= 0xffffffff >> (32 - this->bits_left);
			}
		}
	}

	return TRUE;
}

METHOD(xof_bitspender_t, get_byte, bool,
	private_xof_bitspender_t *this, uint8_t *byte)
{
	if (this->bytes_left == 0)
	{
		if (!get_next_block(this, this->bytes))
		{
			return FALSE;
		}
		this->bytes_left = 4;
	}
	*byte = this->bytes[4 - this->bytes_left--];

	return TRUE;
}

METHOD(xof_bitspender_t, destroy, void,
	private_xof_bitspender_t *this)
{
	DBG2(DBG_LIB, "%N generated %u octets", ext_out_function_names,
				   this->xof->get_type(this->xof), this->octet_count);
	memwipe(this->bytes, 4);
	this->xof->destroy(this->xof);
	free(this);
}

/**
 * See header.
 */
xof_bitspender_t *xof_bitspender_create(ext_out_function_t alg, chunk_t seed,
										bool hash_seed)
{
	private_xof_bitspender_t *this;
	xof_t *xof;

	xof = lib->crypto->create_xof(lib->crypto, alg);
	if (!xof)
	{
		return NULL;
	}

	switch (alg)
	{
		case XOF_MGF1_SHA1:
		case XOF_MGF1_SHA256:
		case XOF_MGF1_SHA512:
		{
			mgf1_t *mgf1 = (mgf1_t*)xof;

			mgf1->set_hash_seed(mgf1, hash_seed);
			break;
		}
		default:
			break;
	}
	if (!xof->set_seed(xof, seed))
	{
		xof->destroy(xof);
		return NULL;
	}
	DBG2(DBG_LIB, "%N is seeded with %u octets", ext_out_function_names,
				   alg, seed.len);

	INIT(this,
		.public = {
			.get_bits = _get_bits,
			.get_byte = _get_byte,
			.destroy = _destroy,
		},
		.xof = xof,
	);

	return &this->public;
}
