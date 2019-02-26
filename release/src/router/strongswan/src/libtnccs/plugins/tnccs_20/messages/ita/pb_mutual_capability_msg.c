/*
 * Copyright (C) 2015 Andreas Steffen
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

#include "pb_mutual_capability_msg.h"

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

ENUM(pb_tnc_mutual_protocol_type_names, PB_MUTUAL_HALF_DUPLEX,
										PB_MUTUAL_FULL_DUPLEX,
	"half duplex",
	"full duplex"
);

typedef struct private_pb_mutual_capability_msg_t private_pb_mutual_capability_msg_t;

/**
 *   PB-Mutual-Capability message
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |H|F|                      Reserved                             |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

# define MUTUAL_CAPABILITY_HEADER_SIZE		4

/**
 * Private data of a pb_mutual_capability_msg_t object.
 *
 */
struct private_pb_mutual_capability_msg_t {
	/**
	 * Public pb_mutual_capability_msg_t interface.
	 */
	pb_mutual_capability_msg_t public;

	/**
	 * PB-TNC message type
	 */
	pen_type_t type;

	/**
	 * PB-TNC mutual protocols
	 */
	uint32_t protocols;

	/**
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(pb_tnc_msg_t, get_type, pen_type_t,
	private_pb_mutual_capability_msg_t *this)
{
	return this->type;
}

METHOD(pb_tnc_msg_t, get_encoding, chunk_t,
	private_pb_mutual_capability_msg_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_msg_t, build, void,
	private_pb_mutual_capability_msg_t *this)
{
	bio_writer_t *writer;

	if (this->encoding.ptr)
	{
		return;
	}
	writer = bio_writer_create(MUTUAL_CAPABILITY_HEADER_SIZE);
	writer->write_uint32(writer, this->protocols);

	this->encoding = writer->get_buf(writer);
	this->encoding = chunk_clone(this->encoding);
	writer->destroy(writer);
}

METHOD(pb_tnc_msg_t, process, status_t,
	private_pb_mutual_capability_msg_t *this, uint32_t *offset)
{
	bio_reader_t *reader;

	*offset = 0;

	/* process message */
	reader = bio_reader_create(this->encoding);
	reader->read_uint32(reader, &this->protocols);
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pb_tnc_msg_t, destroy, void,
	private_pb_mutual_capability_msg_t *this)
{
	free(this->encoding.ptr);
	free(this);
}

METHOD(pb_mutual_capability_msg_t, get_protocols, uint32_t,
	private_pb_mutual_capability_msg_t *this)
{
	return this->protocols;
}

/**
 * See header
 */
pb_tnc_msg_t* pb_mutual_capability_msg_create(uint32_t protocols)
{
	private_pb_mutual_capability_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_protocols = _get_protocols,
		},
		.type = { PEN_ITA, PB_ITA_MSG_MUTUAL_CAPABILITY },
		.protocols = protocols,
	);

	return &this->public.pb_interface;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_mutual_capability_msg_create_from_data(chunk_t data)
{
	private_pb_mutual_capability_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_protocols = _get_protocols,
		},
		.type = { PEN_ITA, PB_ITA_MSG_MUTUAL_CAPABILITY },
		.encoding = chunk_clone(data),
	);

	return &this->public.pb_interface;
}

