/*
 * Copyright (C) 2010 Andreas Steffen
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

#include "pb_remediation_parameters_msg.h"

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

ENUM(pb_tnc_remed_param_type_names, PB_REMEDIATION_URI, PB_REMEDIATION_STRING,
	"Remediation-URI",
	"Remediation-String"
);

typedef struct private_pb_remediation_parameters_msg_t private_pb_remediation_parameters_msg_t;

/**
 *   PB-Remediation-Parameters message (see section 4.8 of RFC 5793)
 *
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |    Reserved   |       Remediation Parameters Vendor ID        |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                  Remediation Parameters Type                  |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |            Remediation Parameters (Variable Length)           |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                   Remediation String Length                   |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                Remediation String (Variable Length)           |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     | Lang Code Len |  Remediation String Lang Code (Variable Len)  |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Private data of a pb_remediation_parameters_msg_t object.
 *
 */
struct private_pb_remediation_parameters_msg_t {
	/**
	 * Public pb_remediation_parameters_msg_t interface.
	 */
	pb_remediation_parameters_msg_t public;

	/**
	 * PB-TNC message type
	 */
	pen_type_t type;

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
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(pb_tnc_msg_t, get_type, pen_type_t,
	private_pb_remediation_parameters_msg_t *this)
{
	return this->type;
}

METHOD(pb_tnc_msg_t, get_encoding, chunk_t,
	private_pb_remediation_parameters_msg_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_msg_t, build, void,
	private_pb_remediation_parameters_msg_t *this)
{
	bio_writer_t *writer;

	if (this->encoding.ptr)
	{
		return;
	}
	writer = bio_writer_create(64);
	writer->write_uint32(writer, this->parameters_type.vendor_id);
	writer->write_uint32(writer, this->parameters_type.type);
	writer->write_data  (writer, this->parameters);

	this->encoding = writer->get_buf(writer);
	this->encoding = chunk_clone(this->encoding);
	writer->destroy(writer);
}

METHOD(pb_tnc_msg_t, process, status_t,
	private_pb_remediation_parameters_msg_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	u_int8_t reserved;
	status_t status = SUCCESS;
	u_char *pos;

	*offset = 0;

	/* process message */
	reader = bio_reader_create(this->encoding);
	reader->read_uint8 (reader, &reserved);
	reader->read_uint24(reader, &this->parameters_type.vendor_id);
	reader->read_uint32(reader, &this->parameters_type.type);
	reader->read_data  (reader, reader->remaining(reader), &this->parameters);

	this->parameters = chunk_clone(this->parameters);
	reader->destroy(reader);

	if (this->parameters_type.vendor_id == PEN_IETF &&
		this->parameters_type.type == PB_REMEDIATION_STRING)
	{
		reader = bio_reader_create(this->parameters);
		status = FAILED;
		*offset = 8;

		if (!reader->read_data32(reader, &this->string))
		{
			DBG1(DBG_TNC, "insufficient data for remediation string");
			goto end;
		};
		*offset += 4;

		pos = memchr(this->string.ptr, '\0', this->string.len);
		if (pos)
		{
			DBG1(DBG_TNC, "nul termination in remediation string");
			*offset += (pos - this->string.ptr);
			goto end;
		}
		*offset += this->string.len;

		if (!reader->read_data8(reader, &this->lang_code))
		{
			DBG1(DBG_TNC, "insufficient data for remediation string lang code");
			goto end;
		};
		*offset += 1;

		pos = memchr(this->lang_code.ptr, '\0', this->lang_code.len);

		if (pos)
		{
			DBG1(DBG_TNC, "nul termination in remediation string lang code");
			*offset += (pos - this->lang_code.ptr);
			goto end;
		}
		status = SUCCESS;

end:
		reader->destroy(reader);
	}
	return status;
}

METHOD(pb_tnc_msg_t, destroy, void,
	private_pb_remediation_parameters_msg_t *this)
{
	free(this->encoding.ptr);
	free(this->parameters.ptr);
	free(this);
}

METHOD(pb_remediation_parameters_msg_t, get_parameters_type, pen_type_t,
	private_pb_remediation_parameters_msg_t *this)
{
	return this->parameters_type;
}

METHOD(pb_remediation_parameters_msg_t, get_parameters, chunk_t,
	private_pb_remediation_parameters_msg_t *this)
{
	return this->parameters;
}

METHOD(pb_remediation_parameters_msg_t, get_string, chunk_t,
	private_pb_remediation_parameters_msg_t *this, chunk_t *lang_code)
{
	if (lang_code)
	{
		*lang_code = this->lang_code;
	}
	return this->string;
}

/**
 * See header
 */
pb_tnc_msg_t* pb_remediation_parameters_msg_create(pen_type_t parameters_type,
												   chunk_t parameters)
{
	private_pb_remediation_parameters_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_parameters_type = _get_parameters_type,
			.get_parameters = _get_parameters,
			.get_uri = _get_parameters,
			.get_string = _get_string,
		},
		.type = { PEN_IETF, PB_MSG_REMEDIATION_PARAMETERS },
		.parameters_type = parameters_type,
		.parameters = chunk_clone(parameters),
	);

	return &this->public.pb_interface;
}

/**
 * Described in header.
 */
pb_tnc_msg_t* pb_remediation_parameters_msg_create_from_uri(chunk_t uri)
{
	pen_type_t type = { PEN_IETF, PB_REMEDIATION_URI };

	return pb_remediation_parameters_msg_create(type, uri);
}

/**
 * Described in header.
 */
pb_tnc_msg_t* pb_remediation_parameters_msg_create_from_string(chunk_t string,
															   chunk_t lang_code)
{
	pb_tnc_msg_t *msg;
	bio_writer_t *writer;
	pen_type_t type = { PEN_IETF, PB_REMEDIATION_STRING };

	/* limit language code to 255 octets */
	lang_code.len = min(255, lang_code.len);

	writer = bio_writer_create(4 + string.len + 1 + lang_code.len);
	writer->write_data32(writer, string);
	writer->write_data8 (writer, lang_code);

	msg = pb_remediation_parameters_msg_create(type, writer->get_buf(writer));
	writer->destroy(writer);

	return msg;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_remediation_parameters_msg_create_from_data(chunk_t data)
{
	private_pb_remediation_parameters_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_parameters_type = _get_parameters_type,
			.get_parameters = _get_parameters,
			.get_uri = _get_parameters,
			.get_string = _get_string,
		},
		.type = { PEN_IETF, PB_MSG_REMEDIATION_PARAMETERS },
		.encoding = chunk_clone(data),
	);

	return &this->public.pb_interface;
}

