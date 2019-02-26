/*
 * Copyright (C) 2011 Sansar Choinyambuu, Andreas Steffen
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

#include "pts/components/pts_comp_evidence.h"

#include <utils/debug.h>

typedef struct private_pts_comp_evidence_t private_pts_comp_evidence_t;

/**
 * Private data of a pts_comp_evidence_t object.
 */
struct private_pts_comp_evidence_t {

	/**
	 * Public pts_comp_evidence_t interface.
	 */
	pts_comp_evidence_t public;

	/**
	 * Component Functional Name
	 */
	pts_comp_func_name_t *name;

	/**
	 * Sub-Component Depth
	 */
	uint32_t depth;

	/**
	 * Measurement Time
	 */
	time_t measurement_time;

	/**
	 * Measurement Time
	 */
	chunk_t measurement;

	/**
	 * Measurement Hash Algorithm
	 */
	pts_meas_algorithms_t hash_algorithm;

	/**
	 * Is PCR Information included?
	 */
	bool has_pcr_info;

	/**
	 * PCR the measurement was extended into
	 */
	uint32_t extended_pcr;

	/**
	 * PCR value before extension
	 */
	chunk_t pcr_before;

	/**
	 * PCR value after extension
	 */
	chunk_t pcr_after;

	/**
	 * Transformation used for extending measurement into PCR
	 */
	pts_pcr_transform_t transform;

	/**
	 * Component Validation Result
	 */
	pts_comp_evid_validation_t validation;

	/**
	 * Verification Policy URI
	 */
	char *policy_uri;

};

METHOD(pts_comp_evidence_t, get_comp_func_name, pts_comp_func_name_t*,
	private_pts_comp_evidence_t *this, uint32_t *depth)
{
	if (depth)
	{
		*depth = this->depth;
	}
	return this->name;
}

METHOD(pts_comp_evidence_t, get_extended_pcr, uint32_t,
	private_pts_comp_evidence_t *this)
{
	return this->extended_pcr;
}

METHOD(pts_comp_evidence_t, get_measurement, chunk_t,
	private_pts_comp_evidence_t *this, uint32_t *extended_pcr,
	pts_meas_algorithms_t *algo, pts_pcr_transform_t *transform,
	time_t *measurement_time)
{
	if (extended_pcr)
	{
		*extended_pcr = this->extended_pcr;
	}
	if (algo)
	{
		*algo = this->hash_algorithm;
	}
	if (transform)
	{
		*transform = this->transform;
	}
	if (measurement_time)
	{
		*measurement_time = this->measurement_time;
	}
	return this->measurement;
}

METHOD(pts_comp_evidence_t, get_pcr_info, bool,
	private_pts_comp_evidence_t *this, chunk_t *pcr_before, chunk_t *pcr_after)
{
	if (pcr_before)
	{
		*pcr_before = this->pcr_before;
	}
	if (pcr_after)
	{
		*pcr_after = this->pcr_after;
	}
	return this->has_pcr_info;
}

METHOD(pts_comp_evidence_t, set_pcr_info, void,
	private_pts_comp_evidence_t *this, chunk_t pcr_before, chunk_t pcr_after)
{
	this->has_pcr_info = TRUE;
	this->pcr_before = pcr_before;
	this->pcr_after =  pcr_after;

	DBG3(DBG_PTS, "PCR %2d before value : %#B", this->extended_pcr, &pcr_before);
	DBG3(DBG_PTS, "PCR %2d after value  : %#B", this->extended_pcr, &pcr_after);
}

METHOD(pts_comp_evidence_t, get_validation, pts_comp_evid_validation_t,
	private_pts_comp_evidence_t *this, char **uri)
{
	if (uri)
	{
		*uri = this->policy_uri;
	}
	return this->validation;
}

METHOD(pts_comp_evidence_t, set_validation, void,
	private_pts_comp_evidence_t *this, pts_comp_evid_validation_t validation,
	char *uri)
{
	this->validation = validation;
	if (uri)
	{
		this->policy_uri = strdup(uri);
		DBG3(DBG_PTS, "'%s'", uri);
	}
}

METHOD(pts_comp_evidence_t, destroy, void,
	private_pts_comp_evidence_t *this)
{
	this->name->destroy(this->name);
	free(this->measurement.ptr);
	free(this->pcr_before.ptr);
	free(this->pcr_after.ptr);
	free(this->policy_uri);
	free(this);
}

/**
 * See header
 */
pts_comp_evidence_t *pts_comp_evidence_create(pts_comp_func_name_t *name,
											  uint32_t depth,
											  uint32_t extended_pcr,
											  pts_meas_algorithms_t algo,
											  pts_pcr_transform_t transform,
											  time_t measurement_time,
											  chunk_t measurement)
{
	private_pts_comp_evidence_t *this;

	INIT(this,
		.public = {
			.get_comp_func_name = _get_comp_func_name,
			.get_extended_pcr = _get_extended_pcr,
			.get_measurement = _get_measurement,
			.get_pcr_info = _get_pcr_info,
			.set_pcr_info = _set_pcr_info,
			.get_validation = _get_validation,
			.set_validation = _set_validation,
			.destroy = _destroy,
		},
		.name = name,
		.depth = depth,
		.extended_pcr = extended_pcr,
		.hash_algorithm = algo,
		.transform = transform,
		.measurement_time = measurement_time,
		.measurement = measurement,
	);

	name->log(name, "");
	DBG3(DBG_PTS, "measurement time: %T", &measurement_time, FALSE);
	DBG3(DBG_PTS, "PCR %2d extended with: %#B", extended_pcr, &measurement);

	return &this->public;
}

/**
 * See header
 */
pts_pcr_transform_t pts_meas_algo_to_pcr_transform(pts_meas_algorithms_t algo,
												   size_t pcr_len)
{
	size_t hash_size;

	hash_size = pts_meas_algo_hash_size(algo);
	if (hash_size == 0)
	{
		return PTS_PCR_TRANSFORM_NO;
	}
	if (hash_size == pcr_len)
	{
		return PTS_PCR_TRANSFORM_MATCH;
	}
	if (hash_size > pcr_len)
	{
		return PTS_PCR_TRANSFORM_LONG;
	}
	return PTS_PCR_TRANSFORM_SHORT;
}

