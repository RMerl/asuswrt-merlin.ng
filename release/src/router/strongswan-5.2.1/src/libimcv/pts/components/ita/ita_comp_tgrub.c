/*
 * Copyright (C) 2011-2012 Andreas Steffen
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

#include "ita_comp_tgrub.h"
#include "ita_comp_func_name.h"

#include "pts/components/pts_component.h"

#include <utils/debug.h>
#include <pen/pen.h>

typedef struct pts_ita_comp_tgrub_t pts_ita_comp_tgrub_t;

/**
 * Private data of a pts_ita_comp_tgrub_t object.
 *
 */
struct pts_ita_comp_tgrub_t {

	/**
	 * Public pts_component_t interface.
	 */
	pts_component_t public;

	/**
	 * Component Functional Name
	 */
	pts_comp_func_name_t *name;

	/**
	 * Sub-component depth
	 */
	u_int32_t depth;

	/**
	 * PTS measurement database
	 */
	pts_database_t *pts_db;

	/**
	 * Reference count
	 */
	refcount_t ref;

};

METHOD(pts_component_t, get_comp_func_name, pts_comp_func_name_t*,
	pts_ita_comp_tgrub_t *this)
{
	return this->name;
}

METHOD(pts_component_t, get_evidence_flags, u_int8_t,
	pts_ita_comp_tgrub_t *this)
{
	return PTS_REQ_FUNC_COMP_EVID_PCR;
}

METHOD(pts_component_t, get_depth, u_int32_t,
	pts_ita_comp_tgrub_t *this)
{
	return this->depth;
}

METHOD(pts_component_t, measure, status_t,
	pts_ita_comp_tgrub_t *this, u_int8_t qualifier, pts_t *pts,
	pts_comp_evidence_t **evidence)
{
	size_t pcr_len;
	pts_pcr_transform_t pcr_transform;
	pts_meas_algorithms_t hash_algo;
	pts_comp_evidence_t *evid;
	u_int32_t extended_pcr;
	time_t measurement_time;
	chunk_t measurement, pcr_before, pcr_after;

	/* Provisional implementation for TGRUB */
	extended_pcr = PCR_DEBUG;
	time(&measurement_time);

	if (!pts->read_pcr(pts, extended_pcr, &pcr_after))
	{
		DBG1(DBG_PTS, "error occurred while reading PCR: %d", extended_pcr);
		return FAILED;
	}

	hash_algo = PTS_MEAS_ALGO_SHA1;
	pcr_len = HASH_SIZE_SHA1;
	pcr_transform = pts_meas_algo_to_pcr_transform(hash_algo, pcr_len);

	measurement = chunk_alloc(pcr_len);
	memset(measurement.ptr, 0x00, measurement.len);

	pcr_before = chunk_alloc(pcr_len);
	memset(pcr_before.ptr, 0x00, pcr_before.len);

	evid = *evidence = pts_comp_evidence_create(this->name->clone(this->name),
								this->depth, extended_pcr,
								hash_algo, pcr_transform,
								measurement_time, measurement);
	evid->set_pcr_info(evid, pcr_before, pcr_after);

	return SUCCESS;
}

METHOD(pts_component_t, verify, status_t,
	pts_ita_comp_tgrub_t *this, u_int8_t qualifier, pts_t *pts,
	pts_comp_evidence_t *evidence)
{
	bool has_pcr_info;
	u_int32_t extended_pcr;
	pts_meas_algorithms_t algo;
	pts_pcr_transform_t transform;
	pts_pcr_t *pcrs;
	time_t measurement_time;
	chunk_t pcr_before, pcr_after;
	chunk_t measurement __attribute__((unused));

	pcrs = pts->get_pcrs(pts);
	measurement = evidence->get_measurement(evidence, &extended_pcr,
								&algo, &transform, &measurement_time);
	if (extended_pcr != PCR_DEBUG)
	{
		return FAILED;
	}

	/* TODO check measurement in database */

	has_pcr_info = evidence->get_pcr_info(evidence, &pcr_before, &pcr_after);
	if (has_pcr_info)
	{
		if (!chunk_equals(pcr_before, pcrs->get(pcrs, extended_pcr)))
		{
			DBG1(DBG_PTS, "PCR %2u: pcr_before is not equal to pcr value");
		}
		if (pcrs->set(pcrs, extended_pcr, pcr_after))
		{
			return SUCCESS;
		}
	}

	return SUCCESS;
}

METHOD(pts_component_t, finalize, bool,
	pts_ita_comp_tgrub_t *this, u_int8_t qualifier, bio_writer_t *result)
{
	return FALSE;
}

METHOD(pts_component_t, get_ref, pts_component_t*,
	pts_ita_comp_tgrub_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(pts_component_t, destroy, void,
	pts_ita_comp_tgrub_t *this)
{
	if (ref_put(&this->ref))
	{
		this->name->destroy(this->name);
		free(this);
	}
}

/**
 * See header
 */
pts_component_t *pts_ita_comp_tgrub_create(u_int32_t depth,
										   pts_database_t *pts_db)
{
	pts_ita_comp_tgrub_t *this;

	INIT(this,
		.public = {
			.get_comp_func_name = _get_comp_func_name,
			.get_evidence_flags = _get_evidence_flags,
			.get_depth = _get_depth,
			.measure = _measure,
			.verify = _verify,
			.finalize = _finalize,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.name = pts_comp_func_name_create(PEN_ITA, PTS_ITA_COMP_FUNC_NAME_TGRUB,
										  PTS_ITA_QUALIFIER_FLAG_KERNEL |
										  PTS_ITA_QUALIFIER_TYPE_TRUSTED),
		.depth = depth,
		.pts_db = pts_db,
		.ref = 1,
	);

	return &this->public;
}
