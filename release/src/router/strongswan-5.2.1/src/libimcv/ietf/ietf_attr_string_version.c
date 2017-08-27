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

#include "ietf_attr_string_version.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_ietf_attr_string_version_t private_ietf_attr_string_version_t;

/**
 * PA-TNC String Version type  (see section 4.2.4 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Version Len  |   Product Version Number (Variable Length)    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Build Num Len |   Internal Build Number (Variable Length)     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Config. Len  | Configuration Version Number (Variable Length)|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define STRING_VERSION_MIN_SIZE		3

/**
 * Private data of an ietf_attr_string_version_t object.
 */
struct private_ietf_attr_string_version_t {

	/**
	 * Public members of ietf_attr_string_version_t
	 */
	ietf_attr_string_version_t public;

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
	 * Product Version Number
	 */
	chunk_t version;

	/**
	 * Internal Build Number
	 */
	chunk_t build;

	/**
	 * Configuration Version Number
	 */
	chunk_t config;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_attr_string_version_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_attr_string_version_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_attr_string_version_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_attr_string_version_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_attr_string_version_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}

	writer = bio_writer_create(STRING_VERSION_MIN_SIZE);
	writer->write_data8(writer, this->version);
	writer->write_data8(writer, this->build);
	writer->write_data8(writer, this->config);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_attr_string_version_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	status_t status = FAILED;
	chunk_t version, build, config;
	u_char *pos;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < STRING_VERSION_MIN_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for IETF string version");
		return FAILED;
	}
	reader = bio_reader_create(this->value);

	if (!reader->read_data8(reader, &version))
	{
		DBG1(DBG_TNC, "insufficient data for IETF product version number");
		goto end;

	}
	pos = memchr(version.ptr, '\0', version.len);
	if (pos)
	{
		DBG1(DBG_TNC, "nul termination in IETF product version number");
		*offset += 1 + (pos - version.ptr);
		goto end;
	}
	*offset += 1 + version.len;

	if (!reader->read_data8(reader, &build))
	{
		DBG1(DBG_TNC, "insufficient data for IETF internal build number");
		goto end;

	}
	pos = memchr(build.ptr, '\0', build.len);
	if (pos)
	{
		DBG1(DBG_TNC, "nul termination in IETF internal build number");
		*offset += 1 + (pos - build.ptr);
		goto end;
	}
	*offset += 1 + build.len;

	if (!reader->read_data8(reader, &config))
	{
		DBG1(DBG_TNC, "insufficient data for IETF configuration version number");
		goto end;

	}
	pos = memchr(config.ptr, '\0', config.len);
	if (pos)
	{
		DBG1(DBG_TNC, "nul termination in IETF configuration version number");
		*offset += 1 + (pos - config.ptr);
		goto end;
	}

	this->version = chunk_clone(version);
	this->build = chunk_clone(build);
	this->config = chunk_clone(config);
	status = SUCCESS;

end:
	reader->destroy(reader);
	return status;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ietf_attr_string_version_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_attr_string_version_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_attr_string_version_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->version.ptr);
		free(this->build.ptr);
		free(this->config.ptr);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(ietf_attr_string_version_t, get_version, chunk_t,
	private_ietf_attr_string_version_t *this, chunk_t *build, chunk_t *config)
{
	if (build)
	{
		*build = this->build;
	}
	if (config)
	{
		*config = this->config;
	}
	return this->version;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_string_version_create(chunk_t version, chunk_t build,
											   chunk_t config)
{
	private_ietf_attr_string_version_t *this;

	/* limit version numbers to 255 octets */
	version.len = min(255, version.len);
	build.len = min(255, build.len);
	config.len = min(255, config.len);

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
		},
		.type = { PEN_IETF, IETF_ATTR_STRING_VERSION },
		.version = chunk_clone(version),
		.build = chunk_clone(build),
		.config = chunk_clone(config),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_string_version_create_from_data(size_t length,
														 chunk_t data)
{
	private_ietf_attr_string_version_t *this;

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
		},
		.type = { PEN_IETF, IETF_ATTR_STRING_VERSION },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

