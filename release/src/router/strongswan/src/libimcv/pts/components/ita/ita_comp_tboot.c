/*
 * Copyright (C) 2011-2015 Andreas Steffen
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

#include "ita_comp_tboot.h"
#include "ita_comp_func_name.h"

#include "imcv.h"
#include "pts/components/pts_component.h"

#include <utils/debug.h>
#include <pen/pen.h>

typedef struct pts_ita_comp_tboot_t pts_ita_comp_tboot_t;

/**
 * Private data of a pts_ita_comp_tboot_t object.
 *
 */
struct pts_ita_comp_tboot_t {

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
	uint32_t depth;

	/**
	 * PTS measurement database
	 */
	pts_database_t *pts_db;

	/**
	 * Primary key for AIK database entry
	 */
	int aik_id;

	/**
	 * Primary key for Component Functional Name database entry
	 */
	int cid;

	/**
	 * Component is registering measurements
	 */
	bool is_registering;

	/**
	 * Time of TBOOT measurement
	 */
	time_t measurement_time;

	/**
	 * Expected measurement count
	 */
	int count;

	/**
	 * Measurement sequence number
	 */
	int seq_no;

	/**
	 * Reference count
	 */
	refcount_t ref;

};

METHOD(pts_component_t, get_comp_func_name, pts_comp_func_name_t*,
	pts_ita_comp_tboot_t *this)
{
	return this->name;
}

METHOD(pts_component_t, get_evidence_flags, uint8_t,
	pts_ita_comp_tboot_t *this)
{
	return PTS_REQ_FUNC_COMP_EVID_PCR;
}

METHOD(pts_component_t, get_depth, uint32_t,
	pts_ita_comp_tboot_t *this)
{
	return this->depth;
}

METHOD(pts_component_t, measure, status_t,
	pts_ita_comp_tboot_t *this, uint8_t qualifier, pts_t *pts,
	pts_comp_evidence_t **evidence)

{
	size_t pcr_len;
	pts_pcr_t *pcrs;
	pts_pcr_transform_t pcr_transform;
	pts_meas_algorithms_t hash_algo;
	pts_comp_evidence_t *evid;
	char *meas_hex, *pcr_before_hex, *pcr_after_hex;
	chunk_t measurement, pcr_before, pcr_after;
	uint32_t extended_pcr;

	switch (this->seq_no++)
	{
		case 0:
			/* dummy data since currently the TBOOT log is not retrieved */
			time(&this->measurement_time);
			meas_hex = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.pcr17_meas", NULL, lib->ns);
			pcr_before_hex = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.pcr17_before", NULL, lib->ns);
			pcr_after_hex = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.pcr17_after", NULL, lib->ns);
			extended_pcr = PCR_TBOOT_POLICY;
			break;
		case 1:
			/* dummy data since currently the TBOOT log is not retrieved */
			meas_hex = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.pcr18_meas", NULL, lib->ns);
			pcr_before_hex = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.pcr18_before", NULL, lib->ns);
			pcr_after_hex = lib->settings->get_str(lib->settings,
						"%s.plugins.imc-attestation.pcr18_after", NULL, lib->ns);
			extended_pcr = PCR_TBOOT_MLE;
			break;
		default:
			return FAILED;
	}

	if (meas_hex == NULL || pcr_before_hex == NULL || pcr_after_hex == NULL)
	{
		return FAILED;
	}

	hash_algo = PTS_MEAS_ALGO_SHA1;
	pcr_len = HASH_SIZE_SHA1;
	pcr_transform = pts_meas_algo_to_pcr_transform(hash_algo, pcr_len);

	/* get and check the measurement data */
	measurement = chunk_from_hex(
					chunk_create(meas_hex, strlen(meas_hex)), NULL);
	pcr_before = chunk_from_hex(
					chunk_create(pcr_before_hex, strlen(pcr_before_hex)), NULL);
	pcr_after = chunk_from_hex(
					chunk_create(pcr_after_hex, strlen(pcr_after_hex)), NULL);
	if (pcr_before.len != pcr_len || pcr_after.len != pcr_len ||
		measurement.len != pcr_len)
	{
		DBG1(DBG_PTS, "TBOOT measurement or PCR data have the wrong size");
		free(measurement.ptr);
		free(pcr_before.ptr);
		free(pcr_after.ptr);
		return FAILED;
	}

	pcrs = pts->get_pcrs(pts);
	pcrs->set(pcrs, extended_pcr, pcr_after);
	evid = *evidence = pts_comp_evidence_create(this->name->clone(this->name),
							this->depth, extended_pcr, hash_algo, pcr_transform,
							this->measurement_time, measurement);
	evid->set_pcr_info(evid, pcr_before, pcr_after);

	return (this->seq_no < 2) ? NEED_MORE : SUCCESS;
}

METHOD(pts_component_t, verify, status_t,
	pts_ita_comp_tboot_t *this, uint8_t qualifier,pts_t *pts,
	pts_comp_evidence_t *evidence)
{
	bool has_pcr_info;
	uint32_t extended_pcr, vid, name;
	enum_name_t *names;
	pts_meas_algorithms_t algo;
	pts_pcr_transform_t transform;
	pts_pcr_t *pcrs;
	time_t measurement_time;
	chunk_t measurement, pcr_before, pcr_after;
	status_t status;

	this->aik_id = pts->get_aik_id(pts);
	pcrs = pts->get_pcrs(pts);
	measurement = evidence->get_measurement(evidence, &extended_pcr,
								&algo, &transform, &measurement_time);

	status = this->pts_db->get_comp_measurement_count(this->pts_db,
									this->name, this->aik_id, algo,
									&this->cid, &this->count);
	if (status != SUCCESS)
	{
		return status;
	}
	vid = this->name->get_vendor_id(this->name);
	name = this->name->get_name(this->name);
	names = imcv_pts_components->get_comp_func_names(imcv_pts_components, vid);

	if (this->count)
	{
		DBG1(DBG_PTS, "checking %d %N '%N' functional component evidence "
			 "measurements", this->count, pen_names, vid, names, name);
	}
	else
	{
		DBG1(DBG_PTS, "registering %N '%N' functional component evidence "
			 "measurements", pen_names, vid, names, name);
		this->is_registering = TRUE;
	}

	if (this->is_registering)
	{
		status = this->pts_db->insert_comp_measurement(this->pts_db,
								measurement, this->cid, this->aik_id,
								++this->seq_no, extended_pcr, algo);
		if (status != SUCCESS)
		{
			return status;
		}
		this->count = this->seq_no + 1;
	}
	else
	{
		status = this->pts_db->check_comp_measurement(this->pts_db,
								measurement, this->cid, this->aik_id,
								++this->seq_no, extended_pcr, algo);
		if (status != SUCCESS)
		{
			return status;
		}
	}

	has_pcr_info = evidence->get_pcr_info(evidence, &pcr_before, &pcr_after);
	if (has_pcr_info)
	{
		if (!chunk_equals_const(pcr_before, pcrs->get(pcrs, extended_pcr)))
		{
			DBG1(DBG_PTS, "PCR %2u: pcr_before is not equal to register value",
						   extended_pcr);
		}
		if (pcrs->set(pcrs, extended_pcr, pcr_after))
		{
			return SUCCESS;
		}
	}

	return SUCCESS;
}

METHOD(pts_component_t, finalize, bool,
	pts_ita_comp_tboot_t *this, uint8_t qualifier, bio_writer_t *result)
{
	char result_buf[BUF_LEN];

	if (this->is_registering)
	{
		/* close registration */
		this->is_registering = FALSE;

		snprintf(result_buf, BUF_LEN, "registered %d evidence measurements",
				 this->seq_no);
	}
	else if (this->seq_no < this->count)
	{
		snprintf(result_buf, BUF_LEN, "%d of %d evidence measurements "
				 "missing", this->count - this->seq_no, this->count);
		return FALSE;
	}
	else
	{
		snprintf(result_buf, BUF_LEN, "%d evidence measurements are ok",
				 this->count);
	}
	DBG1(DBG_PTS, "%s", result_buf);
	result->write_data(result, chunk_from_str(result_buf));

	return TRUE;
}

METHOD(pts_component_t, get_ref, pts_component_t*,
	pts_ita_comp_tboot_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(pts_component_t, destroy, void,
	   pts_ita_comp_tboot_t *this)
{
	int count;
	uint32_t vid, name;
	enum_name_t *names;

	if (ref_put(&this->ref))
	{
		if (this->is_registering)
		{
			count = this->pts_db->delete_comp_measurements(this->pts_db,
												this->cid, this->aik_id);
			vid = this->name->get_vendor_id(this->name);
			name = this->name->get_name(this->name);
			names = imcv_pts_components->get_comp_func_names(imcv_pts_components,
												vid);
			DBG1(DBG_PTS, "deleted %d registered %N '%N' functional component "
				 "evidence measurements", count, pen_names, vid, names, name);
		}
		this->name->destroy(this->name);
		free(this);
	}
}

/**
 * See header
 */
pts_component_t *pts_ita_comp_tboot_create(uint32_t depth,
										   pts_database_t *pts_db)
{
	pts_ita_comp_tboot_t *this;

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
		.name = pts_comp_func_name_create(PEN_ITA, PTS_ITA_COMP_FUNC_NAME_TBOOT,
										  PTS_ITA_QUALIFIER_FLAG_KERNEL |
										  PTS_ITA_QUALIFIER_TYPE_TRUSTED),
		.depth = depth,
		.pts_db = pts_db,
		.ref = 1,
	);

	return &this->public;
}
