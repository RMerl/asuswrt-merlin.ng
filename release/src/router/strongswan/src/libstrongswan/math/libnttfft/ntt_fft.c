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

#include "ntt_fft.h"
#include "ntt_fft_reduce.h"

/**
 * Described in header.
 */
void libnttfft_init(void)
{
	/* empty */
}

typedef struct private_ntt_fft_t private_ntt_fft_t;

/**
 * Private data structure for ntt_fft_t object
 */
struct private_ntt_fft_t {

	/**
	 * Public interface.
	 */
	ntt_fft_t public;

	/**
	 * FFT parameter set used as constants
	 */
	const ntt_fft_params_t *p;

};

METHOD(ntt_fft_t, get_size, uint16_t,
	private_ntt_fft_t *this)
{
	return this->p->n;
}

METHOD(ntt_fft_t, get_modulus, uint16_t,
	private_ntt_fft_t *this)
{
	return this->p->q;
}

/**
 * Do an FFT butterfly operation
 *
 * x[i1] ---|+|------- x[i1]
 *        \/
 *        /\    w[iw]  
 * x[i2] ---|-|--|*|-- x[i2]
 *
 */
static void butterfly(private_ntt_fft_t *this, uint32_t *x, int i1,int i2, int iw)
{
	uint32_t xp, xm;

	xp = x[i1] + x[i2];
	xm = x[i1] + (this->p->q - x[i2]);
	if (xp >= this->p->q)
	{
		xp -= this->p->q;
	}
	x[i1] = xp;
	x[i2] = ntt_fft_mreduce(xm * this->p->wr[iw], this->p);
}

/**
 * Trivial butterfly operation of last FFT stage
 */
static void butterfly_last(private_ntt_fft_t *this, uint32_t *x, int i1)
{
	uint32_t xp, xm;
	int i2 = i1 + 1;

	xp = x[i1] + x[i2];
	xm = x[i1] + (this->p->q - x[i2]);
	if (xp >= this->p->q)
	{
		xp -= this->p->q;
	}
	if (xm >= this->p->q)
	{
		xm -= this->p->q;
	}
	x[i1] = xp;
	x[i2] = xm;
}

METHOD(ntt_fft_t, transform, void,
	private_ntt_fft_t *this, uint32_t *a, uint32_t *b, bool inverse)
{
	int stage, i, j, k, m, n, s, t, iw, i_rev;
	uint32_t tmp;

	/* we are going to use the transform size n a lot */
	n = this->p->n;
	s = this->p->s;

	if (!inverse)
	{
		/* apply linear phase needed for negative wrapped convolution */
		for (i = 0; i < n; i++)
		{
			b[i] = ntt_fft_mreduce(a[i] * this->p->wf[s*i], this->p);
		}
	}
	else if (a != b)
	{
		/* copy if input and output array are not the same */
		for (i = 0; i < n; i++)
		{
			b[i] = a[i];
		}
	}

	m = n;
	k = 1;

	for (stage = this->p->stages; stage > 0; stage--)
	{
		m >>= 1;
		t = 0;

		for (j = 0; j < k; j++)
		{
			if (stage == 1)
			{
				butterfly_last(this, b, t);
			}
			else
			{
				for (i = 0; i < m; i++)
				{
					iw = s * (inverse ? (n - i * k) : (i * k));
					butterfly(this, b, t + i, t + i + m, iw);
				}				
			}
			t += 2*m;
		}
		k <<= 1;
	}

	/* Sort output in bit-reverse order */
	for (i = 0; i < n; i++)
	{
		i_rev = this->p->rev[i];

		if (i_rev > i)
		{
			tmp = b[i];
			b[i] = b[i_rev];
			b[i_rev] = tmp;
		}
	}

	/**
	 * Compensate the linear phase needed for negative wrapped convolution
	 * and normalize the output array with 1/n mod q after the inverse FFT. 
	 */
	if (inverse)
	{
		for (i = 0; i < n; i++)
		{
			b[i] = ntt_fft_mreduce(b[i] * this->p->wi[i], this->p);
		}
	}
}

METHOD(ntt_fft_t, destroy, void,
	private_ntt_fft_t *this)
{
	free(this);
}

/**
 * See header.
 */
ntt_fft_t *ntt_fft_create(const ntt_fft_params_t *params)
{
	private_ntt_fft_t *this;

	INIT(this,
		.public = {
			.get_size = _get_size,
			.get_modulus = _get_modulus,
			.transform = _transform,
			.destroy = _destroy,
		},
		.p = params,
	);

	return &this->public;
}
