/*
 * Copyright (C) 2016 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Based on public domain code by Erdem Alkim, Léo Ducas, Thomas Pöppelmann,
 * and Peter Schwabe.
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
 * 
 */

#include "newhope_reconciliation.h"

typedef struct private_newhope_reconciliation_t private_newhope_reconciliation_t;

/**
 * Private data of an newhope_reconciliation_t object.
 */
struct private_newhope_reconciliation_t {

	/**
	 * Public newhope_reconciliation_t interface.
	 */
	newhope_reconciliation_t public;

	/**
	 * Array sizes
	 */
	int n, n4;

	/**
	 * Multiples of modulus q
	 */
	int32_t q, q2, q4, q8, q16;
};


static inline int32_t rec_abs(int32_t v)
{
  int32_t mask = v >> 31;

  return (v ^ mask) - mask;
}

/**
 * Auxiliary function used by help_reconcile() method
 */
static int32_t rec_f(private_newhope_reconciliation_t *this,
					 int32_t v, uint8_t r, int32_t *v0, int32_t *v1)
{
	int32_t x, xit, t, b;

	x = 8 * v + 2 * r;

	/* compute t = x/q */
	b = x * 2730;
	t = b >> 25;
	b = x - t * this->q;
	b = this->q - 1 - b;
	b >>= 31;
	t -= b;

	r = t & 0x01;
	xit = (t >> 1);
	*v0 = xit + r ; /* v0 = round(x/(2q)) */

	t -= 1;
	r = t & 0x01;
	*v1 = ( t>> 1) + r;

	return rec_abs(x - (*v0) * this->q2);
}

/**
 * Auxiliary function used by reconcile() method
 */
static int32_t rec_g(private_newhope_reconciliation_t *this, int32_t x)
{
	int32_t t, r, b;

	/*  t = x/(4*q) */
	b = x * 2730;
	t = b >> 27;
	b = x - t * this->q4;
	b = this->q4 - 1 - b;
	b >>= 31;
	t -= b;

	r = t & 0x01;
	t = (t >> 1) + r; /* t = round(x/(8q)) */
	t *= this->q8;

  return abs(t - x);
}

METHOD(newhope_reconciliation_t, help_reconcile, uint8_t*,
	private_newhope_reconciliation_t *this, uint32_t *v, uint8_t *rbits)
{
	int32_t v0[4], v1[4], v_tmp[4], k;
	int i, i0, i1, i2, i3, j;
	uint8_t *r, rbit;

	/* allocate output vector */
	r = (uint8_t*)malloc(this->n);

	for (i = 0; i < this->n4/8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			i0 = 8*i  + j;
			i1 = i0 + this->n4;
			i2 = i1 + this->n4;
			i3 = i2 + this->n4;

			/* iterate through all 256 random bits */
			rbit = (rbits[i] >> j) & 0x01;

			k  = rec_f(this, v[i0], rbit, &v0[0], &v1[0]);
			k += rec_f(this, v[i1], rbit, &v0[1], &v1[1]);
			k += rec_f(this, v[i2], rbit, &v0[2], &v1[2]);
			k += rec_f(this, v[i3], rbit, &v0[3], &v1[3]);

			k = (this->q2 - 1 - k) >> 31;

			v_tmp[0] = ((~k) & v0[0]) ^ (k & v1[0]);
			v_tmp[1] = ((~k) & v0[1]) ^ (k & v1[1]);
			v_tmp[2] = ((~k) & v0[2]) ^ (k & v1[2]);
			v_tmp[3] = ((~k) & v0[3]) ^ (k & v1[3]);

			r[i0] = (v_tmp[0] -     v_tmp[3]) & 0x03;
			r[i1] = (v_tmp[1] -     v_tmp[3]) & 0x03;
			r[i2] = (v_tmp[2] -     v_tmp[3]) & 0x03;
			r[i3] = (v_tmp[3] - k + v_tmp[3]) & 0x03;
		}
	}

	return r;
}

METHOD(newhope_reconciliation_t, reconcile, chunk_t,
	private_newhope_reconciliation_t *this, uint32_t *v, uint8_t *r)
{
	size_t key_len;
	uint8_t *key;
	int32_t tmp[4], t;
	int i, i0, i1, i2, i3, j;

	key_len = this->n4 / 8;
	key = (uint8_t*)malloc(key_len);
	memset(key, 0x00, key_len);

	for (i = 0; i < key_len; i++)
	{
		for (j = 0; j < 8; j++)
		{
			i0 = 8*i + j;
			i1 = i0 + this->n4;
			i2 = i1 + this->n4;
			i3 = i2 + this->n4;

			tmp[0] = this->q16 + 8 * (int32_t)v[i0] - 
					 this->q  * (2*r[i0] + r[i3]);
			tmp[1] = this->q16 + 8 * (int32_t)v[i1] -
					 this->q  * (2*r[i1] + r[i3]);
			tmp[2] = this->q16 + 8 * (int32_t)v[i2] -
					 this->q  * (2*r[i2] + r[i3]);
			tmp[3] = this->q16 + 8 * (int32_t)v[i3] -
					 this->q *  (          r[i3]);

			t = rec_g(this, tmp[0]) + rec_g(this, tmp[1]) +
				rec_g(this, tmp[2]) + rec_g(this, tmp[3]) - this->q8;

			key[i] |= ((t >> 31) & 0x01) << j;
		}
	}

	return chunk_create(key, key_len);
}

METHOD(newhope_reconciliation_t, destroy, void,
	private_newhope_reconciliation_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
newhope_reconciliation_t *newhope_reconciliation_create(int n, int32_t q)
{
	private_newhope_reconciliation_t *this;

	INIT(this,
		.public = {
			.help_reconcile = _help_reconcile,
			.reconcile = _reconcile,
			.destroy = _destroy,
		},
		.n   =  n,
		.n4  =  n / 4,
		.q   =      q,
		.q2  =  2 * q,
		.q4  =  4 * q,
		.q8  =  8 * q,
		.q16 = 16 * q,
	);

	return &this->public;
}
