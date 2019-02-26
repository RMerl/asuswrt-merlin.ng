/*
 * Copyright (C) 2017 Andreas Steffen
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

#include "tpm_rng.h"

#include <tpm_tss.h>
#include <utils/debug.h>

typedef struct private_tpm_rng_t private_tpm_rng_t;

/**
 * Private data of an tpm_rng_t object.
 */
struct private_tpm_rng_t {

	/**
	 * Public interface.
	 */
	tpm_rng_t public;

	/**
	 * Trusted Platform Module
	 */
	tpm_tss_t *tpm;

};

METHOD(rng_t, get_bytes, bool,
	private_tpm_rng_t *this, size_t bytes, uint8_t *buffer)
{
	return this->tpm->get_random(this->tpm, bytes, buffer);
}

METHOD(rng_t, allocate_bytes, bool,
	private_tpm_rng_t *this, size_t bytes, chunk_t *chunk)
{
	*chunk = chunk_alloc(bytes);
	if (!get_bytes(this, chunk->len, chunk->ptr))
	{
		chunk_clear(chunk);
		return FALSE;
	}
	return TRUE;
}

METHOD(rng_t, destroy, void,
	private_tpm_rng_t *this)
{
	this->tpm->destroy(this->tpm);
	free(this);
}

/*
 * Described in header.
 */
tpm_rng_t *tpm_rng_create(rng_quality_t quality)
{
	private_tpm_rng_t *this;
	tpm_tss_t *tpm;

	/* try to find a TPM 2.0 */
	tpm = tpm_tss_probe(TPM_VERSION_2_0);
	if (!tpm)
	{
		DBG1(DBG_LIB, "no TPM 2.0 found");
		return NULL;	
	}

	INIT(this,
		.public = {
			.rng = {
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.destroy = _destroy,
			},
		},
		.tpm = tpm,
	);

	return &this->public;
}

