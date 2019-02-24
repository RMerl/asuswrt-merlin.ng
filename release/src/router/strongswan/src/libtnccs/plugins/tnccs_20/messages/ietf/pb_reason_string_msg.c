/*
 * Copyright (C) 2010 Sansar Choinyambuu
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

#include "pb_reason_string_msg.h"

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_pb_reason_string_msg_t private_pb_reason_string_msg_t;

/**
 *   PB-Language-Preference message (see section 4.11 of RFC 5793)
 *
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                      Reason String Length                     |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                Reason String (Variable Length)                |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     | Lang Code Len | Reason String Language Code (Variable Length) |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Private data of a pb_reason_string_msg_t object.
 *
 */
struct private_pb_reason_string_msg_t {
	/**
	 * Public pb_reason_string_msg_t interface.
	 */
	pb_reason_string_msg_t public;

	/**
	 * PB-TNC message type
	 */
	pen_type_t type;

	/**
	 * Reason string
	 */
	chunk_t reason_string;

	/**
	 * Language code
	 */
	chunk_t language_code;

	/**
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(pb_tnc_msg_t, get_type, pen_type_t,
	private_pb_reason_string_msg_t *this)
{
	return this->type;
}

METHOD(pb_tnc_msg_t, get_encoding, chunk_t,
	private_pb_reason_string_msg_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_msg_t, build, void,
	private_pb_reason_string_msg_t *this)
{
	bio_writer_t *writer;

	if (this->encoding.ptr)
	{
		return;
	}
	writer = bio_writer_create(64);
	writer->write_data32(writer, this->reason_string);
	writer->write_data8 (writer, this->language_code);

	this->encoding = writer->get_buf(writer);
	this->encoding = chunk_clone(this->encoding);
	writer->destroy(writer);
}

METHOD(pb_tnc_msg_t, process, status_t,
	private_pb_reason_string_msg_t *this, uint32_t *offset)
{
	bio_reader_t *reader;

	reader = bio_reader_create(this->encoding);
	if (!reader->read_data32(reader, &this->reason_string))
	{
		DBG1(DBG_TNC, "could not parse reason string");
		reader->destroy(reader);
		*offset = 0;
		return FAILED;
	};
	this->reason_string = chunk_clone(this->reason_string);

	if (this->reason_string.len &&
		this->reason_string.ptr[this->reason_string.len-1] == '\0')
	{
		DBG1(DBG_TNC, "reason string must not be null terminated");
		reader->destroy(reader);
		*offset = 3 + this->reason_string.len;
		return FAILED;
	}

	if (!reader->read_data8(reader, &this->language_code))
	{
		DBG1(DBG_TNC, "could not parse language code");
		reader->destroy(reader);
		*offset = 4 + this->reason_string.len;
		return FAILED;
	};
	this->language_code = chunk_clone(this->language_code);
	reader->destroy(reader);

	if (this->language_code.len &&
		this->language_code.ptr[this->language_code.len-1] == '\0')
	{
		DBG1(DBG_TNC, "language code must not be null terminated");
		*offset = 4 + this->reason_string.len + this->language_code.len;
		return FAILED;
	}

	return SUCCESS;
}

METHOD(pb_tnc_msg_t, destroy, void,
	private_pb_reason_string_msg_t *this)
{
	free(this->encoding.ptr);
	free(this->reason_string.ptr);
	free(this->language_code.ptr);
	free(this);
}

METHOD(pb_reason_string_msg_t, get_reason_string, chunk_t,
	private_pb_reason_string_msg_t *this)
{
	return this->reason_string;
}

METHOD(pb_reason_string_msg_t, get_language_code, chunk_t,
	private_pb_reason_string_msg_t *this)
{
	return this->language_code;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_reason_string_msg_create_from_data(chunk_t data)
{
	private_pb_reason_string_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_reason_string = _get_reason_string,
			.get_language_code = _get_language_code,
		},
		.type = { PEN_IETF, PB_MSG_REASON_STRING },
		.encoding = chunk_clone(data),
	);

	return &this->public.pb_interface;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_reason_string_msg_create(chunk_t reason_string,
										  chunk_t language_code)
{
	private_pb_reason_string_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_reason_string = _get_reason_string,
			.get_language_code = _get_language_code,
		},
		.type = { PEN_IETF, PB_MSG_REASON_STRING },
		.reason_string = chunk_clone(reason_string),
		.language_code = chunk_clone(language_code),
	);

	return &this->public.pb_interface;
}
