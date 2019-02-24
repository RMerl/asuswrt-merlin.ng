/*
 * Copyright (C) 2012-2014 Andreas Steffen
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

#include "ietf_attr_op_status.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

#include <time.h>

typedef struct private_ietf_attr_op_status_t private_ietf_attr_op_status_t;

ENUM(op_status_names, OP_STATUS_UNKNOWN, OP_STATUS_OPERATIONAL,
	"unknown",
	"not installed",
	"installed",
	"operational"
);

ENUM(op_result_names, OP_RESULT_UNKNOWN, OP_RESULT_UNSUCCESSFUL,
	"unknown",
	"successful",
	"errored",
	"unsuccessful"
);

/**
 * PA-TNC Operational Status type  (see section 4.2.5 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Status     |     Result    |         Reserved              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          Last Use                             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     Last Use (continued)                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     Last Use (continued)                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     Last Use (continued)                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     Last Use (continued)                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define OP_STATUS_SIZE		24

/**
 * Private data of an ietf_attr_op_status_t object.
 */
struct private_ietf_attr_op_status_t {

	/**
	 * Public members of ietf_attr_op_status_t
	 */
	ietf_attr_op_status_t public;

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
	 * Status
	 */
	uint8_t status;

	/**
	 * Result
	 */
	uint8_t result;

	/**
	 * Last Use
	 */
	time_t last_use;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_attr_op_status_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_attr_op_status_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_attr_op_status_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_attr_op_status_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_attr_op_status_t *this)
{
	bio_writer_t *writer;
	char last_use[24];
	struct tm t;

	if (this->value.ptr)
	{
		return;
	}

	/* Conversion from time_t to RFC 3339 ASCII string */
	gmtime_r(&this->last_use, &t);
	snprintf(last_use, 21, "%04d-%02d-%02dT%02d:%02d:%02dZ", 1900 + t.tm_year,
			 t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

	writer = bio_writer_create(OP_STATUS_SIZE);
	writer->write_uint8 (writer, this->status);
	writer->write_uint8 (writer, this->result);
	writer->write_uint16(writer, 0x0000);
	writer->write_data  (writer, chunk_create(last_use, 20));

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_attr_op_status_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	chunk_t last_use;
	uint16_t reserved;
	struct tm t;
	char buf[BUF_LEN];

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len != OP_STATUS_SIZE)
	{
		DBG1(DBG_TNC, "incorrect size for IETF operational status");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint8 (reader, &this->status);
	reader->read_uint8 (reader, &this->result);
	reader->read_uint16(reader, &reserved);
	reader->read_data  (reader, 20, &last_use);
	reader->destroy(reader);

	if (this->status > OP_STATUS_ROOF)
	{
		DBG1(DBG_TNC, "invalid status value %c for IETF operational status",
					   this->status);
		return FAILED;
	}

	*offset = 1;

	if (this->result > OP_RESULT_ROOF)
	{
		DBG1(DBG_TNC, "invalid result value %c for IETF operational status",
					   this->result);
		return FAILED;
	}

	*offset = 4;

	/* Conversion from RFC 3339 ASCII string to time_t */
	snprintf(buf, sizeof(buf), "%.*s", (int)last_use.len, last_use.ptr);
	if (sscanf(buf, "%4d-%2d-%2dT%2d:%2d:%2dZ", &t.tm_year, &t.tm_mon,
			   &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec) != 6)
	{
		DBG1(DBG_TNC, "invalid last_use time format in IETF operational status");
		return FAILED;
	}
	t.tm_year -= 1900;
	t.tm_mon -= 1;
	t.tm_isdst = 0;
	this->last_use = mktime(&t) - timezone;

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ietf_attr_op_status_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_attr_op_status_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_attr_op_status_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this);
	}
}

METHOD(ietf_attr_op_status_t, get_status, uint8_t,
	private_ietf_attr_op_status_t *this)
{
	return this->status;
}

METHOD(ietf_attr_op_status_t, get_result, uint8_t,
	private_ietf_attr_op_status_t *this)
{
	return this->result;
}

METHOD(ietf_attr_op_status_t, get_last_use, time_t,
	private_ietf_attr_op_status_t *this)
{
	return this->last_use;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_op_status_create(uint8_t status, uint8_t result,
										  time_t last_use)
{
	private_ietf_attr_op_status_t *this;

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
			.get_status = _get_status,
			.get_result = _get_result,
			.get_last_use = _get_last_use,
		},
		.type = { PEN_IETF, IETF_ATTR_OPERATIONAL_STATUS },
		.status = status,
		.result = result,
		.last_use = last_use,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_op_status_create_from_data(size_t length, chunk_t data)
{
	private_ietf_attr_op_status_t *this;

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
			.get_status = _get_status,
			.get_result = _get_result,
			.get_last_use = _get_last_use,
		},
		.type = { PEN_IETF, IETF_ATTR_OPERATIONAL_STATUS },
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

