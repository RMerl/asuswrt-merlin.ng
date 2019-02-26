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
 */

#include "newhope_ke.h"
#include "newhope_noise.h"
#include "newhope_reconciliation.h"

#include <ntt_fft.h>
#include <ntt_fft_reduce.h>
#include <crypto/diffie_hellman.h>
#include <utils/debug.h>

static const int seed_len =   32;  /* 256 bits */
static const int poly_len = 1792;  /* size of 1024 packed 14-bit coefficients */
static const int rec_len =   256;  /* size of 1024 packed  2-bit coefficients */

typedef struct private_newhope_ke_t private_newhope_ke_t;

/**
 * Private data of an newhope_ke_t object.
 */
struct private_newhope_ke_t {

	/**
	 * Public newhope_ke_t interface.
	 */
	newhope_ke_t public;

	/**
	 * FFT parameter set
	 */
	const ntt_fft_params_t *params;

	/**
	 * Secret noise polynomial s
	 */
	uint32_t *s;

	/**
	 * Output polynomial u = a * NTT(s') + NTT(e')
	 */
	uint32_t *u;

	/**
	 * Error reconciliation help bits
	 */
	uint8_t *r;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;

};

/**
 * Derive 14-bit coefficients of polynomial a from 256 bit random seed
 * using the SHAKE128 extended output function
 */
static uint32_t* derive_a_poly(private_newhope_ke_t *this, chunk_t seed)
{
	uint32_t *a;
	uint8_t x[2];
	int i = 0;
	xof_t *xof;

	xof = lib->crypto->create_xof(lib->crypto, XOF_SHAKE_128);
	if (!xof)
	{
		DBG1(DBG_LIB, "could not instantiate SHAKE128 XOF");
		return NULL;
	}

	if (!xof->set_seed(xof, seed))
	{
		DBG1(DBG_LIB, "could not set seed of SHAKE128 XOF");
		xof->destroy(xof);
		return NULL;
	}

	/* allocate dynamic memory for polynomial a */
	a = (uint32_t*)malloc(this->params->n * sizeof(uint32_t));

	while (i < this->params->n)
	{
		if (!xof->get_bytes(xof, sizeof(x), x))
		{
			DBG1(DBG_LIB, "could not get bytes from SHAKE128 XOF");
			xof->destroy(xof);
			free(a);
			return NULL;
		}

		/*
		 * Treat x as a 16 bit unsigned little endian integer
		 * and truncate to 14 bits
		 */
		a[i] = uletoh16(x) & 0x3fff;

		if (a[i] < this->params->q)
		{
			i++;
		}
	}
	xof->destroy(xof);

	return a;
}

/**
 * Pack four 14-bit coefficients into seven consecutive bytes
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |L 0 0 0 0 0 0 0|L 1 H 0 0 0 0 0|M 1 1 1 1 1 1 1|L 2 2 2 H 1 1 1|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |M 2 2 2 2 2 2 2|L 3 3 3 3 3 H 2|H 3 3 3 3 3 3 3|L 0 0 0 0 0 0 0|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
static void pack_poly(private_newhope_ke_t *this, uint8_t *x, uint32_t *p)
{
	int i;

	for (i = 0; i < this->params->n; i += 4)
	{
		*x++ = (p[i] & 0xff );
		*x++ = (p[i]   >>  8) | (p[i+1] << 6);
		*x++ = (p[i+1] >>  2);
		*x++ = (p[i+1] >> 10) | (p[i+2] << 4);
		*x++ = (p[i+2] >>  4);
		*x++ = (p[i+2] >> 12) | (p[i+3] << 2);
		*x++ = (p[i+3] >>  6);
	}
}

/**
 * Unpack seven consecutive bytes into four 14-bit coefficients
 */
static uint32_t* unpack_poly(private_newhope_ke_t * this, uint8_t *x)
{
	uint32_t *p;
	int i;

	p = (uint32_t*)malloc(this->params->n * sizeof(uint32_t));

	for (i = 0; i < this->params->n; i += 4)
	{
		p[i]   =  x[0]       | (((uint32_t)x[1] & 0x3f) <<  8);
		p[i+1] = (x[1] >> 6) | (((uint32_t)x[2]) <<  2)
							 | (((uint32_t)x[3] & 0x0f) << 10);
		p[i+2] = (x[3] >> 4) | (((uint32_t)x[4]) <<  4)
							 | (((uint32_t)x[5] & 0x03) << 12);
		p[i+3] = (x[5] >> 2) | (((uint32_t)x[6]) <<  6);
		x += 7;
	}
	for (i = 0; i < this->params->n; i++)
	{
		if (p[i] >= this->params->q)
		{
			DBG1(DBG_LIB, "polynomial coefficient must be smaller than %u",
						   this->params->q);
			free(p);
			return NULL;
		}
	}
	return p;
}

/**
 * Multiply and add polynomials in the frequency domain
 */
static uint32_t* multiply_add_poly(private_newhope_ke_t *this,
								   uint32_t *a, uint32_t *e)
{
	ntt_fft_t *fft;
	uint32_t *b, t;
	int i;

	/* transform s and h to frequency domain */
	fft = ntt_fft_create(this->params);
	fft->transform(fft, this->s, this->s, FALSE);
	fft->transform(fft, e, e, FALSE);
	fft->destroy(fft);

	b = (uint32_t*)malloc(this->params->n * sizeof(uint32_t));

	/* compute  b = a * s + e in the frequency domain */
	for (i = 0; i < this->params->n; i++)
	{
		/* convert a[i] to Montgomery domain */
		t = ntt_fft_mreduce(a[i] * this->params->r2, this->params);

		/* compute b[i] = a[i] * s[i] + e[i] in Montgomery domain */
		t = ntt_fft_mreduce(t * this->s[i], this->params) + e[i];

 		/* exit Montgomery domain before transmitting polynomial b */
		b[i] = ntt_fft_mreduce(t, this->params);
	}
	memwipe(e, this->params->n * sizeof(uint32_t));

	return b;
}

/**
 * Multiply polynomials in the frequency domain and return to time domain
 */
static uint32_t* multiply_ntt_inv_poly(private_newhope_ke_t *this, uint32_t *b)
{
	ntt_fft_t *fft;
	uint32_t *v, t;
	int i;

	v = (uint32_t*)malloc(this->params->n * sizeof(uint32_t));

	for (i = 0; i < this->params->n; i++)
	{
		/* convert b[i] to Montgomery domain */
		t = ntt_fft_mreduce(b[i] * this->params->r2, this->params);

		/* compute v[i] = b[i] * s[i] in Montgomery domain */
		v[i] = ntt_fft_mreduce(t * this->s[i], this->params);
	}

	/* transform v back to time domain */
	fft = ntt_fft_create(this->params);
	fft->transform(fft, v, v, TRUE);
	fft->destroy(fft);

	return v;
}

/**
 * Pack four 2-bit coefficients into one byte
 */
static void pack_rec(private_newhope_ke_t *this, uint8_t *x, uint8_t *r)
{
	int i;

	for (i = 0; i < this->params->n; i += 4)
	{
		*x++ = r[i] | r[i+1] << 2 | r[i+2] << 4 | r[i+3] << 6;
	}
}

static uint8_t* unpack_rec(private_newhope_ke_t *this, uint8_t *x)
{
	uint8_t *r;
	int i;

	r = (uint8_t*)malloc(this->params->n);

	for (i = 0; i < this->params->n; i += 4)
	{
		r[i]   = (*x)      & 0x03;
		r[i+1] = (*x >> 2) & 0x03;
		r[i+2] = (*x >> 4) & 0x03;
		r[i+3] = (*x >> 6) & 0x03;
		x++;
	}

	return r;
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_newhope_ke_t *this, chunk_t *value)
{
	uint16_t n, q;
	int i;

	/* Define some often-used constants */
	n = this->params->n;
	q = this->params->q;

	/* are we the initiator? */
	if (this->u == NULL)
	{
		rng_t *rng;
		uint32_t *a = NULL, *b = NULL, *e = NULL;
		uint8_t noise_seed_buf[seed_len];
		chunk_t noise_seed = { noise_seed_buf, seed_len};
		chunk_t a_seed;
		newhope_noise_t *noise = NULL;
		bool success = FALSE;

		/* allocate space for public output value */
		*value = chunk_alloc(poly_len + seed_len);
		a_seed = chunk_create(value->ptr + poly_len, seed_len);

		/* create polynomial a from 256 bit random seed */
		rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
		if (!rng)
		{
			DBG1(DBG_LIB, "could not instantiate random source");
			return FALSE;
		}
		if (!rng->get_bytes(rng, seed_len, a_seed.ptr))
		{
			DBG1(DBG_LIB, "could not generate seed for polynomial a");
			goto end;
		}

		a = derive_a_poly(this, a_seed);
		if (a == NULL)
		{
			goto end;
		}

		/* generate random seed for the derivation of noise polynomials */
		if (!rng->get_bytes(rng, seed_len, noise_seed.ptr))
		{
			DBG1(DBG_LIB, "could not generate seed for noise polynomials");
			goto end;
		}

		/* create noise polynomial generator */
		noise = newhope_noise_create(noise_seed);
		if (!noise)
		{
			goto end;
		}

		/* create noise polynomial s from seed with nonce = 0x00 */
		this->s = noise->get_binomial_words(noise, 0x00, n, q);
		if (this->s == NULL)
		{
			goto end;
		}

		/* create noise polynomial e from seed with nonce = 0x01 */
		e = noise->get_binomial_words(noise, 0x01, n, q);
		if (e == NULL)
		{
			goto end;
		}

		/* compute b = a * NTT(s) + NTT(e) */
		b = multiply_add_poly(this, a, e);

		DBG3(DBG_LIB, "   i  a[i]  b[i]");
		for (i = 0; i < n; i++)
		{
			DBG3(DBG_LIB, "%4d %5u %5u", i, a[i], b[i]);
		}

		/* pack coefficients of polynomial b */
		pack_poly(this, value->ptr, b);
		success = TRUE;

	end:
		rng->destroy(rng);
		DESTROY_IF(noise);
		free(a);
		free(b);
		free(e);

		if (!success)
		{
		chunk_free(value);
		}
		return success;
	}
	else
	{
		DBG3(DBG_LIB, "   i  u[i]  r[i]");
		for (i = 0; i < n; i++)
		{
			DBG3(DBG_LIB, "%4d %5u %5u", i, this->u[i], this->r[i]);
		}

		/* allocate space for public output value */
		*value = chunk_alloc(poly_len + rec_len);

		/* pack coefficients of polynomial u */
		pack_poly(this, value->ptr, this->u);

		/* pack coefficients of polynomial r */
		pack_rec(this, value->ptr + poly_len, this->r);

		return TRUE;
	}
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_newhope_ke_t *this, chunk_t *secret)
{
	if (this->shared_secret.len == 0)
	{
		*secret = chunk_empty;
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);

	return TRUE;
}

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_newhope_ke_t *this, chunk_t value)
{
	newhope_reconciliation_t * rec;
	uint16_t n, q;
	int i;

	/* Define some often-used constants */
	n = this->params->n;
	q = this->params->q;

	/* are we the responder? */
	if (this->s == NULL)
	{
		uint32_t *a = NULL, *b = NULL, *e1 = NULL, *e2 = NULL, *v = NULL, t;
		uint8_t *rbits = NULL;
		uint8_t noise_seed_buf[seed_len];
		chunk_t noise_seed = { noise_seed_buf, seed_len };
		chunk_t a_seed;
		newhope_noise_t *noise = NULL;
		rng_t *rng = NULL;
		bool success = FALSE;

		if (value.len != poly_len + seed_len)
		{
			DBG1(DBG_LIB, "received %N KE payload of incorrect size",
						   diffie_hellman_group_names, NH_128_BIT);
			return FALSE;
		}
		a_seed = chunk_create(value.ptr + poly_len, seed_len);

		a = derive_a_poly(this, a_seed);
		if (a == NULL)
		{
			return FALSE;
		}

		b = unpack_poly(this, value.ptr);
		if (b == NULL)
		{
			goto end;
		}

		/* debug output of polynomials a and b */
		DBG3(DBG_LIB, "   i  a[i]  b[i]");
		for (i = 0; i < n; i++)
		{
			DBG3(DBG_LIB, "%4d %5u %5u", i, a[i], b[i]);
		}

		/* generate random seed for the derivation of noise polynomials */
		rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
		if (!rng)
		{
			DBG1(DBG_LIB, "could not instantiate random source");
			goto end;
		}
		if (!rng->get_bytes(rng, seed_len, noise_seed.ptr))
		{
			DBG1(DBG_LIB, "could not generate seed for noise polynomials");
			goto end;
		}

		/* create noise polynomial generator */
		noise = newhope_noise_create(noise_seed);
		if (!noise)
		{
			goto end;
		}

		/* create noise polynomial s' from seed with nonce = 0x00 */
		this->s = noise->get_binomial_words(noise, 0x00, n, q);
		if (this->s == NULL)
		{
			goto end;
		}

		/* create noise polynomial e' from seed with nonce = 0x01 */
		e1 = noise->get_binomial_words(noise, 0x01, n, q);
		if (e1 == NULL)
		{
			goto end;
		}

		/* create noise polynomial e'' from seed with nonce = 0x02 */
		e2 = noise->get_binomial_words(noise, 0x02, n, q);
		if (e2 == NULL)
		{
			goto end;
		}

		/* compute u = a * NTT(s') + NTT(e') */
		this->u = multiply_add_poly(this, a, e1);

		/* compute v = NTT_inv( b * NTT(s') ) */
		v = multiply_ntt_inv_poly(this, b);

		/* compute v = v + e'' */
		for (i = 0; i < n; i++)
		{
			t = v[i] + e2[i];
			v[i] = (t < q) ? t : t - q;
		}
		memwipe(e2, n * sizeof(uint32_t));

		/* create uniform noise bytes from seed with nonce = 0x02 */
		rbits = noise->get_uniform_bytes(noise, 0x03, n/(4*8));

		rec = newhope_reconciliation_create(n, q);
		this->r = rec->help_reconcile(rec, v, rbits);
		free(rbits);
		this->shared_secret = rec->reconcile(rec, v, this->r);
		rec->destroy(rec);

		DBG4(DBG_LIB, "key: %B", &this->shared_secret);
		success = TRUE;

	end:
		DESTROY_IF(rng);
		DESTROY_IF(noise);
		free(a);
		free(b);
		free(e1);
		free(e2);
		free(v);

		return success;
	}
	else
	{
		uint32_t *v;

		if (value.len != poly_len + rec_len)
		{
			DBG1(DBG_LIB, "received %N KE payload of incorrect size",
						   diffie_hellman_group_names, NH_128_BIT);
			return FALSE;
		}

		this->u = unpack_poly(this, value.ptr);
		if (this->u == NULL)
		{
			return FALSE;
		}

		this->r = unpack_rec(this, value.ptr + poly_len);
		if (this->r == NULL)
		{
			return FALSE;
		}

		DBG3(DBG_LIB, "   i  u[i]  r[i]");
		for (i = 0; i < n; i++)
		{
			DBG3(DBG_LIB, "%4d %5u %5u", i, this->u[i], this->r[i]);
		}

		/* compute v' = NTT_inv( u * NTT(s) ) */
		v = multiply_ntt_inv_poly(this, this->u);

		rec = newhope_reconciliation_create(n, q);
		this->shared_secret = rec->reconcile(rec, v, this->r);
		free(v);
		rec->destroy(rec);

		DBG4(DBG_LIB, "key: %B", &this->shared_secret);

		return TRUE;
	}
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_newhope_ke_t *this)
{
	return NH_128_BIT;
}

METHOD(diffie_hellman_t, destroy, void,
	private_newhope_ke_t *this)
{
	chunk_clear(&this->shared_secret);
	memwipe(this->s, this->params->n * sizeof(uint32_t));
	free(this->s);
	free(this->u);
	free(this->r);
	free(this);
}

/*
 * Described in header.
 */
newhope_ke_t *newhope_ke_create(diffie_hellman_group_t group, chunk_t g, chunk_t p)
{
	private_newhope_ke_t *this;

	INIT(this,
		.public = {
			.dh = {
				.get_shared_secret = _get_shared_secret,
				.set_other_public_value = _set_other_public_value,
				.get_my_public_value = _get_my_public_value,
				.get_dh_group = _get_dh_group,
				.destroy = _destroy,
			},
		},
		.params = &ntt_fft_12289_1024,

	);

	return &this->public;
}
