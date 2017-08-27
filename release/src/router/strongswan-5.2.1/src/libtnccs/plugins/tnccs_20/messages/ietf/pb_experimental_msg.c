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

#include "pb_experimental_msg.h"

typedef struct private_pb_experimental_msg_t private_pb_experimental_msg_t;

/**
 * Private data of a pb_experimental_msg_t object.
 *
 */
struct private_pb_experimental_msg_t {
	/**
	 * Public pb_experimental_msg_t interface.
	 */
	pb_experimental_msg_t public;

	/**
	 * PB-TNC message type
	 */
	pen_type_t type;

	/**
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(pb_tnc_msg_t, get_type, pen_type_t,
	private_pb_experimental_msg_t *this)
{
	return this->type;
}

METHOD(pb_tnc_msg_t, get_encoding, chunk_t,
	private_pb_experimental_msg_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_msg_t, build, void,
	private_pb_experimental_msg_t *this)
{
	/* nothing to do since message contents equal encoding */
}

METHOD(pb_tnc_msg_t, process, status_t,
	private_pb_experimental_msg_t *this, u_int32_t *offset)
{
	return SUCCESS;
}

METHOD(pb_tnc_msg_t, destroy, void,
	private_pb_experimental_msg_t *this)
{
	free(this->encoding.ptr);
	free(this);
}

/**
 * See header
 */
pb_tnc_msg_t *pb_experimental_msg_create_from_data(chunk_t data)
{
	private_pb_experimental_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
		},
		.type = { PEN_IETF, PB_MSG_EXPERIMENTAL },
		.encoding = chunk_clone(data),
	);

	return &this->public.pb_interface;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_experimental_msg_create(chunk_t body)
{
	return pb_experimental_msg_create_from_data(body);
}
