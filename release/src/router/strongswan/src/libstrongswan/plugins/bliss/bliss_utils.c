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

#include "bliss_utils.h"

#include <asn1/asn1.h>
#include <crypto/hashers/hasher.h>
#include <crypto/xofs/xof_bitspender.h>
#include <utils/debug.h>

/**
 * See header.
 */
int32_t bliss_utils_scalar_product(int32_t *x, int32_t *y, int n)
{
	int32_t product = 0;
	int i;

	for (i = 0; i < n; i++)
	{
		product += x[i] * y[i];
	}

	return product;
}

/**
 * See header.
 */
void bliss_utils_round_and_drop(const bliss_param_set_t *set,
								int32_t *x, int16_t *xd)
{
	int32_t factor;
	int i;

	factor = 1 << set->d;

	for (i = 0; i < set->n; i++)
	{
		xd[i] = ((x[i] + (factor >> 1)) / factor) % set->p;
	}
}

/**
 * See header.
 */
bool bliss_utils_generate_c(ext_out_function_t alg, chunk_t data_hash,
							uint16_t *ud, const bliss_param_set_t *set,
							uint16_t *c_indices)
{
	int i, index_trials = 0, index_found = 0;
	bool index_taken[set->n];
	uint32_t index;
	uint8_t *seed_pos;
	chunk_t seed;
	xof_bitspender_t *bitspender;

	seed = chunk_alloca(data_hash.len + set->n * sizeof(uint16_t));

	/* the data hash makes up the first part of the oracle seed */
	memcpy(seed.ptr, data_hash.ptr, data_hash.len);
	seed_pos = seed.ptr + data_hash.len;

	/* followed by the n elements of the ud vector in network order */
	for (i = 0; i < set->n; i++)
	{
		htoun16(seed_pos, ud[i]);
		seed_pos += sizeof(uint16_t);
	}

	bitspender = xof_bitspender_create(alg, seed, FALSE);
	if (!bitspender)
	{
	    return NULL;
	}

	for (i = 0; i < set->n; i++)
	{
		index_taken[i] = FALSE;
	}

	DBG3(DBG_LIB, " i  c_index[i]");
	while (bitspender->get_bits(bitspender, set->n_bits, &index))
	{
		index_trials++;

		if (!index_taken[index])
		{
			DBG3(DBG_LIB, "%2u %8u", index_found, index);
			c_indices[index_found++] = index;
			index_taken[index] = TRUE;

			if (index_found == set->kappa)
			{
				DBG3(DBG_LIB, "%2d  index trials", index_trials);
				bitspender->destroy(bitspender);
				return TRUE;
			}
		}
	}

	bitspender->destroy(bitspender);
	return FALSE;
}

/**
 * See header.
 */
bool bliss_utils_check_norms(const bliss_param_set_t *set,
							 int32_t *z1, int16_t *z2d)
{
	int32_t z2ds[set->n];
	int32_t z1_min, z1_max, norm;
	int16_t z2d_min, z2d_max;
	int i;

	/* some statistics on the values of z1 and z2d */
	z1_min  = z1_max  = z1[0];
	z2d_min = z2d_max = z2d[0];

	for (i = 1; i < set->n; i++)
	{
		if (z1[i] < z1_min)
		{
			z1_min = z1[i];
		}
		else if (z1[i] > z1_max)
		{
			z1_max = z1[i];
		}
		if (z2d[i] < z2d_min)
		{
			z2d_min = z2d[i];
		}
		else if (z2d[i] > z2d_max)
		{
			z2d_max = z2d[i];
		}
	}
	DBG2(DBG_LIB, "z1 = %d..%d, z2d = %d..%d", z1_min, z1_max, z2d_min, z2d_max);

	/* Restriction on infinite norm */
	for (i = 0; i < set->n; i++)
	{
		z2ds[i] = (1 << set->d) * z2d[i];

		if (z1[i] >=  set->B_inf || z2ds[i] >=  set->B_inf ||
			z1[i] <= -set->B_inf || z2ds[i] <= -set->B_inf)
		{
			DBG2(DBG_LIB, "signature rejected due to excessive infinite norm");
			return FALSE;
		}
	}

	/* Restriction on l2-norm */
	norm = bliss_utils_scalar_product(z1, z1, set->n) +
		   bliss_utils_scalar_product(z2ds, z2ds, set->n);

	if (norm >= set->B_l2)
	{
		DBG2(DBG_LIB, "signature rejected due to excessive l2-norm");
		return FALSE;
	}

	return TRUE;
}
