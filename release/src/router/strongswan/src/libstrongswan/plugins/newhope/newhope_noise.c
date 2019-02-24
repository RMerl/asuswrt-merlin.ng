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

#include "newhope_noise.h"

typedef struct private_newhope_noise_t private_newhope_noise_t;

static const int seed_len =   32;  /* 256 bits */
static const int nonce_len =  12;  /*  96 bits */

/**
 * Private data of an newhope_noise_t object.
 */
struct private_newhope_noise_t {

	/**
	 * Public newhope_noise_t interface.
	 */
	newhope_noise_t public;

	/**
	 * 256 bit seed and 96 bit nonce (44 bytes)
	 */
	chunk_t seed;

	/**
	 * ChaCha20 stream
	 */
	xof_t *xof;

};

METHOD(newhope_noise_t, get_uniform_bytes, uint8_t*,
	private_newhope_noise_t *this, uint8_t nonce, uint16_t n)
{
	uint8_t *bytes;

	this->seed.ptr[seed_len] = nonce;
	if (!this->xof->set_seed(this->xof, this->seed))
	{
		DBG1(DBG_LIB, "could not set seed of CHACHA20 XOF");
		return NULL;
	}

	/* allocate dynamic memory for the noise polynomial */
	bytes = (uint8_t*)malloc(n);

	if (!this->xof->get_bytes(this->xof, n, bytes))
	{
		DBG1(DBG_LIB, "could not get bytes from SHAKE128 XOF");
		free(bytes);
		return NULL;
	}

	return bytes;
}

METHOD(newhope_noise_t, get_binomial_words, uint32_t*,
	private_newhope_noise_t *this, uint8_t nonce, uint16_t n, uint16_t q)
{
	uint32_t *np, a, b, d, t;
	uint8_t x[4];
	int i = 0, j;

	this->seed.ptr[seed_len] = nonce;
	if (!this->xof->set_seed(this->xof, this->seed))
	{
		DBG1(DBG_LIB, "could not set seed of CHACHA20 XOF");
		return NULL;
	}

	/* allocate dynamic memory for the noise polynomial */
	np = (uint32_t*)malloc(n * sizeof(uint32_t));

	for (i = 0; i < n; i++)
	{
		if (!this->xof->get_bytes(this->xof, sizeof(x), x))
		{
			DBG1(DBG_LIB, "could not get bytes from SHAKE128 XOF");
			free(np);
			return NULL;
		}

		/* Treat x as a 32 bit unsigned little endian integer */
		t = uletoh32(x);

		/* Compute Psi_16 distribution */
		d = 0;
		for (j = 0; j < 8; j++)
		{
			d += (t >> j) & 0x01010101;
		}
		a = ((d >>  8) & 0xff) + (d & 0xff);
		b = ((d >> 16) & 0xff) + (d >> 24);
		np[i] = (a >= b) ? a - b : a + q - b;
	}

	return np;
}

METHOD(newhope_noise_t, destroy, void,
	private_newhope_noise_t *this)
{
	this->xof->destroy(this->xof);
	chunk_free(&this->seed);
	free(this);
}

/*
 * Described in header.
 */
newhope_noise_t *newhope_noise_create(chunk_t seed)
{
	private_newhope_noise_t *this;
	xof_t *xof;

	if (seed.len != seed_len)
	{
		DBG1(DBG_LIB, "seed for ChaCha20 stream must be 256 bits");
		return NULL;
	}

	xof = lib->crypto->create_xof(lib->crypto, XOF_CHACHA20);
	if (!xof)
	{
		DBG1(DBG_LIB, "could not instantiate ChaCha20 stream");
		return NULL;
	}	

	INIT(this,
		.public = {
			.get_uniform_bytes = _get_uniform_bytes,
			.get_binomial_words = _get_binomial_words,
			.destroy = _destroy,
		},
		.xof = xof,
		.seed = chunk_alloc(seed_len + nonce_len),
	);

	/* initialize seed for ChaCha 20 stream */
	memcpy(this->seed.ptr, seed.ptr, seed_len);
	memset(this->seed.ptr + seed_len, 0x00, nonce_len);

	return &this->public;
}
