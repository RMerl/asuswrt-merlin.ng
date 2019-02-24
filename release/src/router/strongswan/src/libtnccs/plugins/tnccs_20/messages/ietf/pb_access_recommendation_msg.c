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

#include "pb_access_recommendation_msg.h"

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

ENUM(pb_access_recommendation_code_names, PB_REC_ACCESS_ALLOWED, PB_REC_QUARANTINED,
	"Access Allowed",
	"Access Denied",
	"Quarantined"
);

typedef struct private_pb_access_recommendation_msg_t private_pb_access_recommendation_msg_t;

/**
 *   PB-Access-Recommendation message (see section 4.7 of RFC 5793)
 *
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |          Reserved             |   Access Recommendation Code  |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define ACCESS_RECOMMENDATION_RESERVED	 	0x0000
#define ACCESS_RECOMMENDATION_MSG_SIZE		4
/**
 * Private data of a pb_access_recommendation_msg_t object.
 *
 */
struct private_pb_access_recommendation_msg_t {
	/**
	 * Public pb_access_recommendation_msg_t interface.
	 */
	pb_access_recommendation_msg_t public;

	/**
	 * PB-TNC message type
	 */
	pen_type_t type;

	/**
	 * Access recommendation code
	 */
	uint16_t recommendation;

	/**
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(pb_tnc_msg_t, get_type, pen_type_t,
	private_pb_access_recommendation_msg_t *this)
{
	return this->type;
}

METHOD(pb_tnc_msg_t, get_encoding, chunk_t,
	private_pb_access_recommendation_msg_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_msg_t, build, void,
	private_pb_access_recommendation_msg_t *this)
{
	bio_writer_t *writer;

	if (this->encoding.ptr)
	{
		return;
	}
	writer = bio_writer_create(ACCESS_RECOMMENDATION_MSG_SIZE);
	writer->write_uint16(writer, ACCESS_RECOMMENDATION_RESERVED);
	writer->write_uint16(writer, this->recommendation);
	this->encoding = writer->get_buf(writer);
	this->encoding = chunk_clone(this->encoding);
	writer->destroy(writer);
}

METHOD(pb_tnc_msg_t, process, status_t,
	private_pb_access_recommendation_msg_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint16_t reserved;

	reader = bio_reader_create(this->encoding);
	reader->read_uint16(reader, &reserved);
	reader->read_uint16(reader, &this->recommendation);
	reader->destroy(reader);

	if (this->recommendation < PB_REC_ACCESS_ALLOWED ||
		this->recommendation > PB_REC_QUARANTINED)
	{
		DBG1(DBG_TNC, "invalid access recommendation code (%u)",
					   this->recommendation);
		*offset = 2;
		return FAILED;
	}

	return SUCCESS;
}

METHOD(pb_tnc_msg_t, destroy, void,
	private_pb_access_recommendation_msg_t *this)
{
	free(this->encoding.ptr);
	free(this);
}

METHOD(pb_access_recommendation_msg_t, get_access_recommendation, uint16_t,
	private_pb_access_recommendation_msg_t *this)
{
	return this->recommendation;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_access_recommendation_msg_create_from_data(chunk_t data)
{
	private_pb_access_recommendation_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_access_recommendation = _get_access_recommendation,
		},
		.type = { PEN_IETF, PB_MSG_ACCESS_RECOMMENDATION },
		.encoding = chunk_clone(data),
	);

	return &this->public.pb_interface;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_access_recommendation_msg_create(uint16_t recommendation)
{
	private_pb_access_recommendation_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_access_recommendation = _get_access_recommendation,
		},
		.type = { PEN_IETF, PB_MSG_ACCESS_RECOMMENDATION },
		.recommendation = recommendation,
	);

	return &this->public.pb_interface;
}
