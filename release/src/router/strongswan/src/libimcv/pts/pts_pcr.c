/*
 * Copyright (C) 2012 Andreas Steffen
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

#include "pts_pcr.h"

#include <utils/debug.h>

#include <stdarg.h>

typedef struct private_pts_pcr_t private_pts_pcr_t;

/**
 * Private data of a pts_pcr_t object.
 *
 */
struct private_pts_pcr_t {

	/**
	 * Public pts_pcr_t interface.
	 */
	pts_pcr_t public;

	/**
	 * Shadow PCR registers
	 */
	chunk_t pcrs[PTS_PCR_MAX_NUM];

	/**
	 * Number of extended PCR registers
	 */
	uint32_t pcr_count;

	/**
	 * Highest extended PCR register
	 */
	uint32_t pcr_max;

	/**
	 * Bitmap of extended PCR registers
	 */
	uint8_t pcr_select[PTS_PCR_MAX_NUM / 8];

	/**
	 * Hasher used to extend shadow PCRs
	 */
	hasher_t *hasher;

};

METHOD(pts_pcr_t, get_count, uint32_t,
	private_pts_pcr_t *this)
{
	return this->pcr_count;
}

METHOD(pts_pcr_t, select_pcr, bool,
	private_pts_pcr_t *this, uint32_t pcr)
{
	uint32_t i, f;

	if (pcr >= PTS_PCR_MAX_NUM)
	{
		DBG1(DBG_PTS, "PCR %2u: number is larger than maximum of %u",
					   pcr, PTS_PCR_MAX_NUM-1);
		return FALSE;
	}

	/* Determine PCR selection flag */
	i = pcr / 8;
	f = 1 << (pcr - 8*i);

	/* Has this PCR already been selected? */
	if (!(this->pcr_select[i] & f))
	{
		this->pcr_select[i] |= f;
		this->pcr_max = max(this->pcr_max, pcr);
		this->pcr_count++;
	}
	return TRUE;
}

METHOD(pts_pcr_t, get_selection_size, size_t,
	private_pts_pcr_t *this)
{

	/**
	 * A TPM v1.2 has 24 PCR Registers so the bitmask field length
	 * used by TrouSerS is at least 3 bytes
	 */
	return PTS_PCR_MAX_NUM / 8;
}

typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** current PCR */
	uint32_t pcr;
	/** back reference to parent */
	private_pts_pcr_t *pcrs;
} pcr_enumerator_t;

METHOD(enumerator_t, pcr_enumerator_enumerate, bool,
	pcr_enumerator_t *this, va_list args)
{
	uint32_t i, f, *pcr;

	VA_ARGS_VGET(args, pcr);

	while (this->pcr <= this->pcrs->pcr_max)
	{
		/* Determine PCR selection flag */
		i = this->pcr / 8;
		f = 1 << (this->pcr - 8*i);

		/* Assign current PCR to output argument and increase */
		*pcr = this->pcr++;

		/* return if PCR is selected */
		if (this->pcrs->pcr_select[i] & f)
		{
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(pts_pcr_t, create_enumerator, enumerator_t*,
	private_pts_pcr_t *this)
{
	pcr_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _pcr_enumerator_enumerate,
			.destroy = (void*)free,
		},
		.pcrs = this,
	);

	return (enumerator_t*)enumerator;
}

METHOD(pts_pcr_t, get, chunk_t,
	private_pts_pcr_t *this, uint32_t pcr)
{
	return (pcr < PTS_PCR_MAX_NUM) ? this->pcrs[pcr] : chunk_empty;
}

METHOD(pts_pcr_t, set, bool,
	private_pts_pcr_t *this, uint32_t pcr, chunk_t value)
{
	if (value.len != PTS_PCR_LEN)
	{
		DBG1(DBG_PTS, "PCR %2u: value does not fit", pcr);
		return FALSE;
	}
	if (select_pcr(this, pcr))
	{
		memcpy(this->pcrs[pcr].ptr, value.ptr, PTS_PCR_LEN);
		return TRUE;
	}
	return FALSE;
}

METHOD(pts_pcr_t, extend, chunk_t,
	private_pts_pcr_t *this, uint32_t pcr, chunk_t measurement)
{
	if (measurement.len != PTS_PCR_LEN)
	{
		DBG1(DBG_PTS, "PCR %2u: measurement does not fit", pcr);
		return chunk_empty;
	}
	if (!select_pcr(this, pcr))
	{
		return chunk_empty;
	}
	if (!this->hasher->get_hash(this->hasher, this->pcrs[pcr] , NULL) ||
		!this->hasher->get_hash(this->hasher, measurement, this->pcrs[pcr].ptr))
	{
		DBG1(DBG_PTS, "PCR %2u: not extended due to hasher problem", pcr);
		return chunk_empty;
	}
	return this->pcrs[pcr];
}

METHOD(pts_pcr_t, get_composite, tpm_tss_pcr_composite_t*,
	private_pts_pcr_t *this)
{
	tpm_tss_pcr_composite_t *pcr_composite;
	enumerator_t *enumerator;
	uint16_t selection_size;
	uint32_t pcr_field_size, pcr;
	u_char *pos;

	selection_size = get_selection_size(this);
	pcr_field_size = this->pcr_count * PTS_PCR_LEN;

	INIT(pcr_composite,
		.pcr_select    = chunk_alloc(selection_size),
		.pcr_composite = chunk_alloc(pcr_field_size),
	);

	memcpy(pcr_composite->pcr_select.ptr, this->pcr_select, selection_size);
	pos = pcr_composite->pcr_composite.ptr;

	enumerator = create_enumerator(this);
	while (enumerator->enumerate(enumerator, &pcr))
	{
		memcpy(pos, this->pcrs[pcr].ptr, PTS_PCR_LEN);
		pos += PTS_PCR_LEN;
	}
	enumerator->destroy(enumerator);

	return pcr_composite;
}

METHOD(pts_pcr_t, destroy, void,
	private_pts_pcr_t *this)
{
	uint32_t i;

	for (i = 0; i < PTS_PCR_MAX_NUM; i++)
	{
		free(this->pcrs[i].ptr);
	}
	this->hasher->destroy(this->hasher);
	free(this);
}

/**
 * See header
 */
pts_pcr_t *pts_pcr_create(void)
{
	private_pts_pcr_t *this;
	hasher_t *hasher;
	uint32_t i;

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher)
	{
		DBG1(DBG_PTS, "%N hasher could not be created",
			 hash_algorithm_short_names, HASH_SHA1);
		return NULL;
	}

	INIT(this,
		.public = {
			.get_count = _get_count,
			.select_pcr = _select_pcr,
			.get_selection_size = _get_selection_size,
			.create_enumerator = _create_enumerator,
			.get = _get,
			.set = _set,
			.extend = _extend,
			.get_composite = _get_composite,
			.destroy = _destroy,
		},
		.hasher = hasher,
	);

	for (i = 0; i < PTS_PCR_MAX_NUM; i++)
	{
		this->pcrs[i] = chunk_alloc(PTS_PCR_LEN);
		memset(this->pcrs[i].ptr, 0x00, PTS_PCR_LEN);
	}

	return &this->public;
}

