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

#include "ietf_attr_remediation_instr.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_ietf_attr_remediation_instr_t private_ietf_attr_remediation_instr_t;

/**
 * PA-TNC Remediation Instructions type  (see section 4.2.10 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Reserved   |       Remediation Parameters Vendor ID        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                  Remediation Parameters Type                  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |            Remediation Parameters (Variable Length)           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define REMEDIATION_INSTR_MIN_SIZE		8
#define REMEDIATION_INSTR_RESERVED		0x00

/**
 * IETF Remediation Parameters URI type  (see section 4.2.10.1 of RFC 5792)
 *
 *                     1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                 Remediation URI (Variable Length)             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/**
 * IETF Remediation Parameters String type  (see section 4.2.10.2 of RFC 5792)
 *
 *                     1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                   Remediation String Length                   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Remediation String (Variable Length)           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Lang Code Len |  Remediation String Lang Code (Variable Len)  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Private data of an ietf_attr_remediation_instr_t object.
 */
struct private_ietf_attr_remediation_instr_t {

	/**
	 * Public members of ietf_attr_remediation_instr_t
	 */
	ietf_attr_remediation_instr_t public;

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
	 * Remediation Parameters Type
	 */
	pen_type_t parameters_type;

	/**
	 * Remediation Parameters
	 */
	chunk_t parameters;

	/**
	 * Remediation String
	 */
	chunk_t string;

	/**
	 * Remediation Language Code
	 */
	chunk_t lang_code;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_attr_remediation_instr_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_attr_remediation_instr_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_attr_remediation_instr_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_attr_remediation_instr_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_attr_remediation_instr_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}

	writer = bio_writer_create(REMEDIATION_INSTR_MIN_SIZE);
	writer->write_uint8 (writer, REMEDIATION_INSTR_RESERVED);
	writer->write_uint24(writer, this->parameters_type.vendor_id);
	writer->write_uint32(writer, this->parameters_type.type);
	writer->write_data  (writer, this->parameters);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_attr_remediation_instr_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	u_int8_t reserved;
	status_t status = SUCCESS;
	u_char *pos;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < REMEDIATION_INSTR_MIN_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for IETF remediation instructions");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint8 (reader, &reserved);
	reader->read_uint24(reader, &this->parameters_type.vendor_id);
	reader->read_uint32(reader, &this->parameters_type.type);
	reader->read_data  (reader, reader->remaining(reader), &this->parameters);

	this->parameters = chunk_clone(this->parameters);
	reader->destroy(reader);

	if (this->parameters_type.vendor_id == PEN_IETF &&
		this->parameters_type.type == IETF_REMEDIATION_PARAMETERS_STRING)
	{
		reader = bio_reader_create(this->parameters);
		status = FAILED;
		*offset = 8;

		if (!reader->read_data32(reader, &this->string))
		{
			DBG1(DBG_TNC, "insufficient data for IETF remediation string");
			goto end;
		}
		*offset += 4;

		pos = memchr(this->string.ptr, '\0', this->string.len);
		if (pos)
		{
			DBG1(DBG_TNC, "nul termination in IETF remediation string");
			*offset += (pos - this->string.ptr);
			goto end;
		}
		*offset += this->string.len;

		if (!reader->read_data8(reader, &this->lang_code))
		{
			DBG1(DBG_TNC, "insufficient data for IETF remediation lang code");
			goto end;
		}
		status = SUCCESS;

end:
		reader->destroy(reader);
	}
	return status;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ietf_attr_remediation_instr_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_attr_remediation_instr_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_attr_remediation_instr_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->parameters.ptr);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(ietf_attr_remediation_instr_t, get_parameters_type, pen_type_t,
	private_ietf_attr_remediation_instr_t *this)
{
	return this->parameters_type;
}

METHOD(ietf_attr_remediation_instr_t, get_parameters, chunk_t,
	private_ietf_attr_remediation_instr_t *this)
{
	return this->parameters;
}

METHOD(ietf_attr_remediation_instr_t, get_string, chunk_t,
	private_ietf_attr_remediation_instr_t *this, chunk_t *lang_code)
{
	if (lang_code)
	{
		*lang_code = this->lang_code;
	}
	return this->string;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_remediation_instr_create(pen_type_t parameters_type,
												  chunk_t parameters)
{
	private_ietf_attr_remediation_instr_t *this;

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
			.get_parameters_type = _get_parameters_type,
			.get_parameters = _get_parameters,
			.get_uri = _get_parameters,
			.get_string = _get_string,
		},
		.type = { PEN_IETF, IETF_ATTR_REMEDIATION_INSTRUCTIONS },
		.parameters_type = parameters_type,
		.parameters = chunk_clone(parameters),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_remediation_instr_create_from_uri(chunk_t uri)
{
	pen_type_t type = { PEN_IETF, IETF_REMEDIATION_PARAMETERS_URI };

	return ietf_attr_remediation_instr_create(type, uri);
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_remediation_instr_create_from_string(chunk_t string,
															  chunk_t lang_code)
{
	pa_tnc_attr_t *attr;
	bio_writer_t *writer;
	pen_type_t type = { PEN_IETF, IETF_REMEDIATION_PARAMETERS_STRING };

	/* limit language code to 255 octets */
	lang_code.len = min(255, lang_code.len);

	writer = bio_writer_create(4 + string.len + 1 + lang_code.len);
	writer->write_data32(writer, string);
	writer->write_data8 (writer, lang_code);

	attr = ietf_attr_remediation_instr_create(type, writer->get_buf(writer));
	writer->destroy(writer);

	return attr;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_remediation_instr_create_from_data(size_t length,
															chunk_t data)
{
	private_ietf_attr_remediation_instr_t *this;

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
			.get_parameters_type = _get_parameters_type,
			.get_parameters = _get_parameters,
			.get_uri = _get_parameters,
			.get_string = _get_string,
		},
		.type = { PEN_IETF, IETF_ATTR_REMEDIATION_INSTRUCTIONS },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

