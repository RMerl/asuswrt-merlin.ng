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

#include "ietf_attr_numeric_version.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_ietf_attr_numeric_version_t private_ietf_attr_numeric_version_t;

/**
 * PA-TNC Numeric Version type  (see section 4.2.3 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        Major Version Number                   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        Minor Version Number                   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                            Build Number                       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |      Service Pack Major       |      Service Pack Minor       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define NUMERIC_VERSION_SIZE		16

/**
 * Private data of an ietf_attr_numeric_version_t object.
 */
struct private_ietf_attr_numeric_version_t {

	/**
	 * Public members of ietf_attr_numeric_version_t
	 */
	ietf_attr_numeric_version_t public;

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
	 * Major Version Number
	 */
	uint32_t major_version;

	/**
	 * Minor Version Number
	 */
	uint32_t minor_version;

	/**
	 * IBuild Number
	 */
	uint32_t build;

	/**
	 * Service Pack Major Number
	 */
	uint16_t service_pack_major;

	/**
	 * Service Pack Minor Number
	 */
	uint16_t service_pack_minor;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_attr_numeric_version_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_attr_numeric_version_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_attr_numeric_version_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_attr_numeric_version_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_attr_numeric_version_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}

	writer = bio_writer_create(NUMERIC_VERSION_SIZE);
	writer->write_uint32(writer, this->major_version);
	writer->write_uint32(writer, this->minor_version);
	writer->write_uint32(writer, this->build);
	writer->write_uint16(writer, this->service_pack_major);
	writer->write_uint16(writer, this->service_pack_minor);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_attr_numeric_version_t *this, uint32_t *offset)
{
	bio_reader_t *reader;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < NUMERIC_VERSION_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for IETF numeric version");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint32(reader, &this->major_version);
	reader->read_uint32(reader, &this->minor_version);
	reader->read_uint32(reader, &this->build);
	reader->read_uint16(reader, &this->service_pack_major);
	reader->read_uint16(reader, &this->service_pack_minor);
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ietf_attr_numeric_version_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_attr_numeric_version_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_attr_numeric_version_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this);
	}
}

METHOD(ietf_attr_numeric_version_t, get_version, void,
	private_ietf_attr_numeric_version_t *this, uint32_t *major, uint32_t *minor)
{
	if (major)
	{
		*major = this->major_version;
	}
	if (minor)
	{
		*minor = this->minor_version;
	}
}

METHOD(ietf_attr_numeric_version_t, get_build, uint32_t,
	private_ietf_attr_numeric_version_t *this)
{
	return this->build;
}

METHOD(ietf_attr_numeric_version_t, get_service_pack, void,
	private_ietf_attr_numeric_version_t *this, uint16_t *major, uint16_t *minor)
{
	if (major)
	{
		*major = this->service_pack_major;
	}
	if (minor)
	{
		*minor = this->service_pack_minor;
	}
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_numeric_version_create(uint32_t major, uint32_t minor,
												uint32_t build,
												uint16_t service_pack_major,
												uint16_t service_pack_minor)
{
	private_ietf_attr_numeric_version_t *this;

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
			.get_version = _get_version,
			.get_build = _get_build,
			.get_service_pack = _get_service_pack,
		},
		.type = { PEN_IETF, IETF_ATTR_NUMERIC_VERSION },
		.major_version = major,
		.minor_version = minor,
		.build = build,
		.service_pack_major = service_pack_major,
		.service_pack_minor = service_pack_minor,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_numeric_version_create_from_data(size_t length,
														  chunk_t data)
{
	private_ietf_attr_numeric_version_t *this;

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
			.get_version = _get_version,
			.get_build = _get_build,
			.get_service_pack = _get_service_pack,
		},
		.type = { PEN_IETF, IETF_ATTR_NUMERIC_VERSION },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
