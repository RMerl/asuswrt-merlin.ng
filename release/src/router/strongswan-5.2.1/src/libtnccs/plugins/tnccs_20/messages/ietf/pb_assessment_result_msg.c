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

#include "pb_assessment_result_msg.h"

#include <tncifimv.h>

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_pb_assessment_result_msg_t private_pb_assessment_result_msg_t;

/**
 *   PB-Assessment-Result message (see section 4.6 of RFC 5793)
 *
 *                          1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                       Assessment Result                       |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define ASSESSMENT_RESULT_MSG_SIZE	4

/**
 * Private data of a pb_assessment_result_msg_t object.
 *
 */
struct private_pb_assessment_result_msg_t {
	/**
	 * Public pb_assessment_result_msg_t interface.
	 */
	pb_assessment_result_msg_t public;

	/**
	 * PB-TNC message type
	 */
	pen_type_t type;

	/**
	 * Assessment result code
	 */
	u_int32_t assessment_result;

	/**
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(pb_tnc_msg_t, get_type, pen_type_t,
	private_pb_assessment_result_msg_t *this)
{
	return this->type;
}

METHOD(pb_tnc_msg_t, get_encoding, chunk_t,
	private_pb_assessment_result_msg_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_msg_t, build, void,
	private_pb_assessment_result_msg_t *this)
{
	bio_writer_t *writer;

	if (this->encoding.ptr)
	{
		return;
	}
	writer = bio_writer_create(ASSESSMENT_RESULT_MSG_SIZE);
	writer->write_uint32(writer, this->assessment_result);
	this->encoding = writer->get_buf(writer);
	this->encoding = chunk_clone(this->encoding);
	writer->destroy(writer);
}

METHOD(pb_tnc_msg_t, process, status_t,
	private_pb_assessment_result_msg_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;

	reader = bio_reader_create(this->encoding);
	reader->read_uint32(reader, &this->assessment_result);
	reader->destroy(reader);

	if (this->assessment_result < TNC_IMV_EVALUATION_RESULT_COMPLIANT ||
		this->assessment_result > TNC_IMV_EVALUATION_RESULT_DONT_KNOW)
	{
		DBG1(DBG_TNC, "invalid assessment result (%u)",
					   this->assessment_result);
		*offset = 0;
		return FAILED;
	}

	return SUCCESS;
}

METHOD(pb_tnc_msg_t, destroy, void,
	private_pb_assessment_result_msg_t *this)
{
	free(this->encoding.ptr);
	free(this);
}

METHOD(pb_assessment_result_msg_t, get_assessment_result, u_int32_t,
	private_pb_assessment_result_msg_t *this)
{
	return this->assessment_result;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_assessment_result_msg_create_from_data(chunk_t data)
{
	private_pb_assessment_result_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_assessment_result = _get_assessment_result,
		},
		.type = { PEN_IETF, PB_MSG_ASSESSMENT_RESULT },
		.encoding = chunk_clone(data),
	);

	return &this->public.pb_interface;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_assessment_result_msg_create(u_int32_t assessment_result)
{
	private_pb_assessment_result_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_assessment_result = _get_assessment_result,
		},
		.type = { PEN_IETF, PB_MSG_ASSESSMENT_RESULT },
		.assessment_result = assessment_result,
	);

	return &this->public.pb_interface;
}
