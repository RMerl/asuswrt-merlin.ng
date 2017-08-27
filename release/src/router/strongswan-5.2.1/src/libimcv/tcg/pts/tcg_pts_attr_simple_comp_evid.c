/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "tcg_pts_attr_simple_comp_evid.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

#include <time.h>

typedef struct private_tcg_pts_attr_simple_comp_evid_t private_tcg_pts_attr_simple_comp_evid_t;

/**
 * Simple Component Evidence
 * see section 3.15.1 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Flags      |               Sub-Component Depth             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Specific Functional Component                  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Specific Functional Component                  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Measure. Type |               Extended into PCR               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |        Hash Algorithm     | PCR Transform |     Reserved      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    Measurement Date/Time                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    Measurement Date/Time                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    Measurement Date/Time                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    Measurement Date/Time                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    Measurement Date/Time                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Optional Policy URI Length   |  Opt. Verification Policy URI ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~                Optional Verification Policy URI               ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Optional PCR Length        |   Optional PCR Before Value   ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~           Optional PCR Before Value (Variable Length)         ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~           Optional PCR After Value (Variable Length)          ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~           Component Measurement (Variable Length)             ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Specific Functional Component -> Component Functional Name Structure
 * see section 5.1 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Component Functional Name Vendor ID        |Fam| Qualifier |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                   Component Functional Name                   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

#define PTS_SIMPLE_COMP_EVID_SIZE					40
#define PTS_SIMPLE_COMP_EVID_MEAS_TIME_SIZE			20
#define PTS_SIMPLE_COMP_EVID_RESERVED				0x00
#define PTS_SIMPLE_COMP_EVID_FAMILY_MASK			0xC0
#define PTS_SIMPLE_COMP_EVID_VALIDATION_MASK		0x60
#define PTS_SIMPLE_COMP_EVID_MEAS_TYPE				(1<<7)
#define PTS_SIMPLE_COMP_EVID_FLAG_PCR				(1<<7)

static char *utc_undefined_time_str = "0000-00-00T00:00:00Z";

/**
 * Private data of an tcg_pts_attr_simple_comp_evid_t object.
 */
struct private_tcg_pts_attr_simple_comp_evid_t {

	/**
	 * Public members of tcg_pts_attr_simple_comp_evid_t
	 */
	tcg_pts_attr_simple_comp_evid_t public;

	/**
	 * Vendor-specific attribute type
	 */
	pen_type_t type;

	/**
	 * Length of attribute value
	 */
	size_t length;

	/**
	 * Attribute value or segment
	 */
	chunk_t value;

	/**
	 * Noskip flag
	 */
	bool noskip_flag;

	/**
	 * PTS Component Evidence
	 */
	pts_comp_evidence_t *evidence;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_pts_attr_simple_comp_evid_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_pts_attr_simple_comp_evid_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_pts_attr_simple_comp_evid_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_pts_attr_simple_comp_evid_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

/**
 * Convert time_t to Simple Component Evidence UTS string format
 */
void measurement_time_to_utc(time_t measurement_time, chunk_t *utc_time)
{
	struct tm t;

	if (measurement_time == UNDEFINED_TIME)
	{
		utc_time->ptr = utc_undefined_time_str;
	}
	else
	{
		gmtime_r(&measurement_time, &t);
		sprintf(utc_time->ptr, "%04d-%02d-%02dT%02d:%02d:%02dZ",
				t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
				t.tm_hour, t.tm_min, t.tm_sec);
	}
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_simple_comp_evid_t *this)
{
	bio_writer_t *writer;
	bool has_pcr_info;
	char utc_time_buf[25], *policy_uri;
	u_int8_t flags;
	u_int16_t len;
	u_int32_t depth, extended_pcr;
	pts_comp_func_name_t *name;
	pts_meas_algorithms_t hash_algorithm;
	pts_pcr_transform_t transform;
	pts_comp_evid_validation_t validation;
	time_t measurement_time;
	chunk_t measurement, utc_time, pcr_before, pcr_after;

	if (this->value.ptr)
	{
		return;
	}

	/* Extract parameters from comp_evidence_t object */
	name         = this->evidence->get_comp_func_name(this->evidence,
							&depth);
	measurement  = this->evidence->get_measurement(this->evidence,
							&extended_pcr, &hash_algorithm, &transform,
							&measurement_time);
	has_pcr_info = this->evidence->get_pcr_info(this->evidence,
							&pcr_before, &pcr_after);
	validation   = this->evidence->get_validation(this->evidence,
							&policy_uri);

	/* Determine the flags to set*/
	flags = validation;
	if (has_pcr_info)
	{
		flags |= PTS_SIMPLE_COMP_EVID_FLAG_PCR;
	}

	utc_time = chunk_create(utc_time_buf, PTS_SIMPLE_COMP_EVID_MEAS_TIME_SIZE);
	measurement_time_to_utc(measurement_time, &utc_time);

	writer = bio_writer_create(PTS_SIMPLE_COMP_EVID_SIZE);

	writer->write_uint8 (writer, flags);
	writer->write_uint24(writer, depth);
	writer->write_uint24(writer, name->get_vendor_id(name));
	writer->write_uint8 (writer, name->get_qualifier(name));
	writer->write_uint32(writer, name->get_name(name));
	writer->write_uint8 (writer, PTS_SIMPLE_COMP_EVID_MEAS_TYPE);
	writer->write_uint24(writer, extended_pcr);
	writer->write_uint16(writer, hash_algorithm);
	writer->write_uint8 (writer, transform);
	writer->write_uint8 (writer, PTS_SIMPLE_COMP_EVID_RESERVED);
	writer->write_data  (writer, utc_time);

	/* Optional fields */
	if (validation == PTS_COMP_EVID_VALIDATION_FAILED ||
		validation == PTS_COMP_EVID_VALIDATION_PASSED)
	{
		len = strlen(policy_uri);
		writer->write_uint16(writer, len);
		writer->write_data  (writer, chunk_create(policy_uri, len));
	}
	if (has_pcr_info)
	{
		writer->write_uint16(writer, pcr_before.len);
		writer->write_data  (writer, pcr_before);
		writer->write_data  (writer, pcr_after);
	}

	writer->write_data(writer, measurement);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

static const int days[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
static const int tm_leap_1970 = 477;

/**
 * Convert Simple Component Evidence UTS string format to time_t
 */
bool measurement_time_from_utc(time_t *measurement_time, chunk_t utc_time)
{
	int tm_year, tm_mon, tm_day, tm_days, tm_hour, tm_min, tm_sec, tm_secs;
	int tm_leap_4, tm_leap_100, tm_leap_400, tm_leap;

	if (memeq(utc_undefined_time_str, utc_time.ptr, utc_time.len))
	{
		*measurement_time = 0;
		return TRUE;
	}
	if (sscanf(utc_time.ptr, "%4d-%2d-%2dT%2d:%2d:%2dZ",
		&tm_year, &tm_mon, &tm_day, &tm_hour, &tm_min, &tm_sec) != 6)
	{
		return FALSE;
	}

	/* representation of months as 0..11 */
	tm_mon--;

	/* representation of days as 0..30 */
	tm_day--;

	/* number of leap years between last year and 1970? */
	tm_leap_4 = (tm_year - 1) / 4;
	tm_leap_100 = tm_leap_4 / 25;
	tm_leap_400 = tm_leap_100 / 4;
	tm_leap = tm_leap_4 - tm_leap_100 + tm_leap_400 - tm_leap_1970;

	/* if date later then February, is the current year a leap year? */
	if (tm_mon > 1 && (tm_year % 4 == 0) &&
		(tm_year % 100 != 0 || tm_year % 400 == 0))
	{
		tm_leap++;
	}
	tm_days = 365 * (tm_year - 1970) + days[tm_mon] + tm_day + tm_leap;
	tm_secs = 60 * (60 * (24 * tm_days + tm_hour) + tm_min) + tm_sec;

	*measurement_time = tm_secs;
	return TRUE;
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_simple_comp_evid_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	pts_comp_func_name_t *name;
	u_int8_t flags, fam_and_qualifier, qualifier, reserved;
	u_int8_t measurement_type, transform, validation;
	u_int16_t hash_algorithm, len;
	u_int32_t depth, vendor_id, comp_name, extended_pcr;
	chunk_t measurement, utc_time, policy_uri, pcr_before, pcr_after;
	time_t measurement_time;
	bool has_pcr_info = FALSE, has_validation = FALSE;
	status_t status = FAILED;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PTS_SIMPLE_COMP_EVID_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for Simple Component Evidence");
	}
	reader = bio_reader_create(this->value);

	reader->read_uint8 (reader, &flags);
	reader->read_uint24(reader, &depth);
	reader->read_uint24(reader, &vendor_id);
	reader->read_uint8 (reader, &fam_and_qualifier);
	reader->read_uint32(reader, &comp_name);
	reader->read_uint8 (reader, &measurement_type);
	reader->read_uint24(reader, &extended_pcr);
	reader->read_uint16(reader, &hash_algorithm);
	reader->read_uint8 (reader, &transform);
	reader->read_uint8 (reader, &reserved);
	reader->read_data  (reader, PTS_SIMPLE_COMP_EVID_MEAS_TIME_SIZE, &utc_time);

	if (measurement_type != PTS_SIMPLE_COMP_EVID_MEAS_TYPE)
	{
		DBG1(DBG_TNC, "unsupported Measurement Type in "
					  "Simple Component Evidence");
		*offset = 12;
		reader->destroy(reader);
		return FAILED;
	}
	if (!measurement_time_from_utc(&measurement_time, utc_time))
	{
		DBG1(DBG_TNC, "invalid Measurement Time field in "
					  "Simple Component Evidence");
		*offset = 20;
		reader->destroy(reader);
		return FAILED;
	}
	validation = flags & PTS_SIMPLE_COMP_EVID_VALIDATION_MASK;
	qualifier = fam_and_qualifier & ~PTS_SIMPLE_COMP_EVID_FAMILY_MASK;

	/*  Is optional Policy URI field included? */
	if (validation == PTS_COMP_EVID_VALIDATION_FAILED ||
		validation == PTS_COMP_EVID_VALIDATION_PASSED)
	{
		if (!reader->read_uint16(reader, &len))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Component Evidence "
						  "Verification Policy URI Length");
			goto end;
		}
		if (!reader->read_data(reader, len, &policy_uri))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Component Evidence "
						  "Verification Policy URI");
			goto end;
		}
		has_validation = TRUE;
	}

	/*  Are optional PCR value fields included? */
	if (flags & PTS_SIMPLE_COMP_EVID_FLAG_PCR)
	{
		if (!reader->read_uint16(reader, &len))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Component Evidence "
						  "PCR Value length");
			goto end;
		}
		if (!reader->read_data(reader, len, &pcr_before))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Component Evidence "
						  "PCR Before Value");
			goto end;
		}
		if (!reader->read_data(reader, len, &pcr_after))
		{
			DBG1(DBG_TNC, "insufficient data for PTS Simple Component Evidence "
						  "PCR After Value");
			goto end;
		}
		has_pcr_info = TRUE;
	}

	/* Measurement field comes at the very end */
	reader->read_data(reader,reader->remaining(reader), &measurement);
	reader->destroy(reader);

	/* Create Component Functional Name object */
	name = pts_comp_func_name_create(vendor_id, comp_name, qualifier);

	/* Create Component Evidence object */
	measurement = chunk_clone(measurement);
	this->evidence = pts_comp_evidence_create(name, depth, extended_pcr,
											  hash_algorithm, transform,
											  measurement_time, measurement);

	/* Add options */
	if (has_validation)
	{
		char buf[BUF_LEN];
		size_t len;

		len = min(policy_uri.len, BUF_LEN-1);
		memcpy(buf, policy_uri.ptr, len);
		buf[len] = '\0';
		this->evidence->set_validation(this->evidence, validation, buf);
	}
	if (has_pcr_info)
	{
		pcr_before = chunk_clone(pcr_before);
		pcr_after =  chunk_clone(pcr_after);
		this->evidence->set_pcr_info(this->evidence, pcr_before, pcr_after);
	}

	return SUCCESS;

end:
	reader->destroy(reader);
	return status;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_pts_attr_simple_comp_evid_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_pts_attr_simple_comp_evid_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_pts_attr_simple_comp_evid_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->evidence);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(tcg_pts_attr_simple_comp_evid_t, get_comp_evidence, pts_comp_evidence_t*,
	private_tcg_pts_attr_simple_comp_evid_t *this)
{
	return this->evidence;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_simple_comp_evid_create(pts_comp_evidence_t *evid)
{
	private_tcg_pts_attr_simple_comp_evid_t *this;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_comp_evidence = _get_comp_evidence,
		},
		.type = { PEN_TCG, TCG_PTS_SIMPLE_COMP_EVID },
		.evidence = evid,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_simple_comp_evid_create_from_data(size_t length,
															  chunk_t data)
{
	private_tcg_pts_attr_simple_comp_evid_t *this;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_comp_evidence = _get_comp_evidence,
		},
		.type = { PEN_TCG, TCG_PTS_SIMPLE_COMP_EVID },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
