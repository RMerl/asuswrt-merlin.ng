/*
 * Copyright (C) 2014-2016 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2009-2013  Security Innovation
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

#include "ntru_poly.h"

#include <crypto/xofs/xof_bitspender.h>
#include <utils/debug.h>
#include <utils/test.h>

typedef struct private_ntru_poly_t private_ntru_poly_t;
typedef struct indices_len_t indices_len_t;

/**
 * Stores number of +1 and -1 coefficients
 */
struct indices_len_t {
	int p;
	int m;
};

/**
 * Private data of an ntru_poly_t object.
 */
struct private_ntru_poly_t {

	/**
	 * Public ntru_poly_t interface.
	 */
	ntru_poly_t public;

	/**
	 * Ring dimension equal to the number of polynomial coefficients
	 */
	uint16_t N;

	/**
	 * Large modulus
	 */
	uint16_t q;

	/**
	 * Array containing the indices of the non-zero coefficients
	 */
	uint16_t *indices;

	/**
	 * Number of indices of the non-zero coefficients
	 */
	size_t num_indices;

	/**
	 * Number of sparse polynomials
	 */
	int num_polynomials;

	/**
	 * Number of nonzero coefficients for up to 3 sparse polynomials
	 */
	indices_len_t indices_len[3];

};

METHOD(ntru_poly_t, get_size, size_t,
	private_ntru_poly_t *this)
{
	return this->num_indices;
}

METHOD(ntru_poly_t, get_indices, uint16_t*,
	private_ntru_poly_t *this)
{
	return this->indices;
}

/**
  * Multiplication of polynomial a with a sparse polynomial b given by
  * the indices of its +1 and -1 coefficients results in polynomial c.
  * This is a convolution operation
  */
static void ring_mult_i(uint16_t *a, indices_len_t len, uint16_t *indices,
							  uint16_t N, uint16_t mod_q_mask, uint16_t *t,
							  uint16_t *c)
{
	int i, j, k;

	/* initialize temporary array t */
	for (k = 0; k < N; k++)
	{
		t[k] = 0;
	}

	/* t[(i+k)%N] = sum i=0 through N-1 of a[i], for b[k] = -1 */
	for (j = len.p; j < len.p + len.m; j++)
	{
		k = indices[j];
		for (i = 0; k < N; ++i, ++k)
		{
			t[k] += a[i];
		}
		for (k = 0; i < N; ++i, ++k)
		{
			t[k] += a[i];
		}
	}

	/* t[(i+k)%N] = -(sum i=0 through N-1 of a[i] for b[k] = -1) */
	for (k = 0; k < N; k++)
	{
		t[k] = -t[k];
	}

	/* t[(i+k)%N] += sum i=0 through N-1 of a[i] for b[k] = +1 */
	for (j = 0; j < len.p; j++)
	{
		k = indices[j];
		for (i = 0; k < N; ++i, ++k)
		{
			t[k] += a[i];
		}
		for (k = 0; i < N; ++i, ++k)
		{
			t[k] += a[i];
		}
	}

	/* c = (a * b) mod q */
	for (k = 0; k < N; k++)
	{
		c[k] = t[k] & mod_q_mask;
	}
}

METHOD(ntru_poly_t, get_array, void,
	private_ntru_poly_t *this, uint16_t *array)
{
	uint16_t *t, *bi;
	uint16_t mod_q_mask = this->q - 1;
	indices_len_t len;
	int i;

	/* form polynomial F or F1 */
	memset(array, 0x00, this->N * sizeof(uint16_t));
	bi = this->indices;
	len = this->indices_len[0];
	for (i = 0; i < len.p + len.m; i++)
	{
		array[bi[i]] = (i < len.p) ? 1 : mod_q_mask;
	}

	if (this->num_polynomials == 3)
	{
		/* allocate temporary array t */
		t = malloc(this->N * sizeof(uint16_t));

		/* form F1 * F2 */
		bi += len.p + len.m;
		len = this->indices_len[1];
		ring_mult_i(array, len, bi, this->N, mod_q_mask, t, array);

		/* form (F1 * F2) + F3 */
		bi += len.p + len.m;
		len = this->indices_len[2];
		for (i = 0; i < len.p + len.m; i++)
		{
			if (i < len.p)
			{
				array[bi[i]] += 1;
			}
			else
			{
				array[bi[i]] -= 1;
			}
			array[bi[i]] &= mod_q_mask;
		}
		free(t);
	}
}

METHOD(ntru_poly_t, ring_mult, void,
	private_ntru_poly_t *this, uint16_t *a, uint16_t *c)
{
	uint16_t *t1, *t2;
	uint16_t *bi = this->indices;
	uint16_t mod_q_mask = this->q - 1;
	int i;

	/* allocate temporary array t1 */
	t1 = malloc(this->N * sizeof(uint16_t));

	if (this->num_polynomials == 1)
	{
		ring_mult_i(a, this->indices_len[0], bi, this->N, mod_q_mask, t1, c);
	}
	else
	{
		/* allocate temporary array t2 */
		t2 = malloc(this->N * sizeof(uint16_t));

		/* t1 = a * b1 */
		ring_mult_i(a, this->indices_len[0], bi, this->N, mod_q_mask, t1, t1);

		/* t1 = (a * b1) * b2 */
		bi += this->indices_len[0].p + this->indices_len[0].m;
		ring_mult_i(t1, this->indices_len[1], bi, this->N, mod_q_mask, t2, t1);

		/* t2 = a * b3 */
		bi += this->indices_len[1].p + this->indices_len[1].m;
		ring_mult_i(a, this->indices_len[2], bi, this->N, mod_q_mask, t2, t2);

		/* c = (a * b1 * b2) + (a * b3) */
		for (i = 0; i < this->N; i++)
		{
			c[i] = (t1[i] + t2[i]) & mod_q_mask;
		}
		free(t2);
	}
	free(t1);
}

METHOD(ntru_poly_t, destroy, void,
	private_ntru_poly_t *this)
{
	memwipe(this->indices, sizeof(uint16_t) * get_size(this));
	free(this->indices);
	free(this);
}

/**
 * Creates an empty ntru_poly_t object with space allocated for indices
 */
static private_ntru_poly_t* ntru_poly_create(uint16_t N, uint16_t q,
											 uint32_t indices_len_p,
											 uint32_t indices_len_m,
											 bool is_product_form)
{
	private_ntru_poly_t *this;
	int n;

	INIT(this,
		.public = {
			.get_size = _get_size,
			.get_indices = _get_indices,
			.get_array = _get_array,
			.ring_mult = _ring_mult,
			.destroy = _destroy,
		},
		.N = N,
		.q = q,
	);

	if (is_product_form)
	{
		this->num_polynomials = 3;
		for (n = 0; n < 3; n++)
		{
			this->indices_len[n].p = 0xff & indices_len_p;
			this->indices_len[n].m = 0xff & indices_len_m;
			this->num_indices += this->indices_len[n].p +
								 this->indices_len[n].m;
			indices_len_p >>= 8;
			indices_len_m >>= 8;
		}
	}
	else
	{
		this->num_polynomials = 1;
		this->indices_len[0].p = indices_len_p;
		this->indices_len[0].m = indices_len_m;
		this->num_indices = indices_len_p + indices_len_m;
	}
	this->indices = malloc(sizeof(uint16_t) * this->num_indices);

	return this;
}

/*
 * Described in header.
 */
ntru_poly_t *ntru_poly_create_from_seed(ext_out_function_t mgf1_type,
										chunk_t seed, uint8_t c_bits,
										uint16_t N, uint16_t q,
										uint32_t indices_len_p,
										uint32_t indices_len_m,
										bool is_product_form)
{
	private_ntru_poly_t *this;
	int n, num_indices, index_i = 0;
	uint32_t index, limit;
	uint8_t *used;
	xof_bitspender_t *bitspender;

	bitspender = xof_bitspender_create(mgf1_type, seed, TRUE);
	if (!bitspender)
	{
	    return NULL;
	}
	this = ntru_poly_create(N, q, indices_len_p, indices_len_m, is_product_form);
	used = malloc(N);
	limit = N * ((1 << c_bits) / N);

	/* generate indices for all polynomials */
	for (n = 0; n < this->num_polynomials; n++)
	{
		memset(used, 0, N);
		num_indices = this->indices_len[n].p + this->indices_len[n].m;

		/* generate indices for a single polynomial */
		while (num_indices)
		{
			/* generate a random candidate index with a size of c_bits */		
			do
			{
				if (!bitspender->get_bits(bitspender, c_bits, &index))
				{
					bitspender->destroy(bitspender);
					destroy(this);
					free(used);
					return NULL;
				}
			}
			while (index >= limit);

			/* form index and check if unique */
			index %= N;
			if (!used[index])
			{
				used[index] = 1;
				this->indices[index_i++] = index;
				num_indices--;
			}
		}
	}

	bitspender->destroy(bitspender);
	free(used);

	return &this->public;
}

/*
 * Described in header.
 */
ntru_poly_t *ntru_poly_create_from_data(uint16_t *data, uint16_t N, uint16_t q,
										uint32_t indices_len_p,
										uint32_t indices_len_m,
										bool is_product_form)
{
	private_ntru_poly_t *this;
	int i;

	this = ntru_poly_create(N, q, indices_len_p, indices_len_m, is_product_form);

	for (i = 0; i < this->num_indices; i++)
	{
		this->indices[i] = data[i];
	}

	return &this->public;
}

EXPORT_FUNCTION_FOR_TESTS(ntru, ntru_poly_create_from_seed);

EXPORT_FUNCTION_FOR_TESTS(ntru, ntru_poly_create_from_data);
