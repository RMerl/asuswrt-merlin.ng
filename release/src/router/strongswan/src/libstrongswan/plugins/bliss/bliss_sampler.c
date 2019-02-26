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

#include "bliss_sampler.h"

typedef struct private_bliss_sampler_t private_bliss_sampler_t;

#include <crypto/xofs/xof_bitspender.h>

/**
 * Private data of a bliss_sampler_t object.
 */
struct private_bliss_sampler_t {

	/**
	 * Public interface.
	 */
	bliss_sampler_t public;

	/**
	 * BLISS parameter the rejection sampling is to be based on
	 */
	const bliss_param_set_t *set;

	/**
	 * Bitspender used for random rejection sampling
	 */
	xof_bitspender_t *bitspender;

};

METHOD(bliss_sampler_t, bernoulli_exp, bool,
	private_bliss_sampler_t *this, uint32_t x, bool *accepted)
{
	uint32_t x_mask;
	uint8_t u;
	const uint8_t *c;
	int i;

	x_mask = 1 << (this->set->c_rows - 1);
	c = this->set->c;
	c += (this->set->c_rows - 1) * this->set->c_cols;

	while (x_mask > 0)
	{
		if (x & x_mask)
		{
			for (i = 0; i < this->set->c_cols; i++)
			{
				if (!this->bitspender->get_byte(this->bitspender, &u))
				{
					return FALSE;
				}
				if (u < c[i])
				{
					break;
				}
				else if (u > c[i])
				{
					*accepted = FALSE;
					return TRUE;
				}
			}
		}
		x_mask >>= 1;
		c -= this->set->c_cols;
	}

	*accepted = TRUE;
	return TRUE;
}

METHOD(bliss_sampler_t, bernoulli_cosh, bool,
	private_bliss_sampler_t *this, int32_t x, bool *accepted)
{
	uint32_t u;

	x = 2 * (x < 0 ? -x : x);

	while (TRUE)
	{
		if (!bernoulli_exp(this, x, accepted))
		{
			return FALSE;
		}
		if (*accepted)
		{
			return TRUE;
		}
		if (!this->bitspender->get_bits(this->bitspender, 1, &u))
		{
			return FALSE;
		}
		if (u)
		{
			continue;
		}
		if (!bernoulli_exp(this, x, accepted))
		{
			return FALSE;
		}
		if (!(*accepted))
		{
			return TRUE;
		}
	}
}

#define MAX_SAMPLE_INDEX	16

METHOD(bliss_sampler_t, pos_binary, bool,
	private_bliss_sampler_t *this, uint32_t *x)
{
	uint32_t u, i;

	while (TRUE)
	{
		for (i = 0; i <= MAX_SAMPLE_INDEX; i++)
		{
			if (!this->bitspender->get_bits(this->bitspender,
											i ? (2*i - 1) : 1, &u))
			{
				return FALSE;
			}
			if (u == 0)
			{
				*x = i;
				return TRUE;
			}
			if ((u >> 1) != 0)
			{
				break;
			}
		}
		if (i > MAX_SAMPLE_INDEX)
		{
			return FALSE;
		}
	}
}

METHOD(bliss_sampler_t, gaussian, bool,
	private_bliss_sampler_t *this, int32_t *z)
{
	uint32_t u, x, y, z_pos;
	bool accepted;

	while (TRUE)
	{
		if (!pos_binary(this, &x))
		{
			return FALSE;
		}

		do
		{
			if (!this->bitspender->get_bits(this->bitspender,
											this->set->k_sigma_bits, &y))
			{
				return FALSE;
			}
		}
		while (y >= this->set->k_sigma);

		if (!bernoulli_exp(this, y * (y + 2*this->set->k_sigma * x), &accepted))
		{
			return FALSE;
		}
		if (accepted)
		{
			if (!this->bitspender->get_bits(this->bitspender, 1, &u))
			{
				return FALSE;
			}
			if (x || y || u)
			{ 
				break;
			}
		}
	}

	z_pos = this->set->k_sigma * x + y;
	*z = u ? z_pos : -z_pos;

	return TRUE;
}

METHOD(bliss_sampler_t, sign, bool,
	private_bliss_sampler_t *this, bool *positive)
{
	uint32_t u;

	if (!this->bitspender->get_bits(this->bitspender, 1, &u))
	{
		return FALSE;
	}
	*positive = u;

	return TRUE;
}

METHOD(bliss_sampler_t, destroy, void,
	private_bliss_sampler_t *this)
{
	this->bitspender->destroy(this->bitspender);
	free(this);
}


/**
 * See header.
 */
bliss_sampler_t *bliss_sampler_create(ext_out_function_t alg, chunk_t seed,
									  const bliss_param_set_t *set)
{
	private_bliss_sampler_t *this;
	xof_bitspender_t *bitspender;

	bitspender = xof_bitspender_create(alg, seed, FALSE);
	if (!bitspender)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.bernoulli_exp = _bernoulli_exp,
			.bernoulli_cosh = _bernoulli_cosh,
			.pos_binary = _pos_binary,
			.gaussian = _gaussian,
			.sign = _sign,
			.destroy = _destroy,
		},
		.set = set,
		.bitspender = bitspender,
	);

	return &this->public;
}
