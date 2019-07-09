/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "xpc_logger.h"

typedef struct private_xpc_logger_t private_xpc_logger_t;

/**
 * Private data of an xpc_logger_t object.
 */
struct private_xpc_logger_t {

	/**
	 * Public xpc_logger_t interface.
	 */
	xpc_logger_t public;

	/**
	 * XPC channel to send logging messages to
	 */
	xpc_connection_t conn;

	/**
	 * IKE_SA we log for
	 */
	uint32_t ike_sa;
};

METHOD(logger_t, log_, void,
	private_xpc_logger_t *this, debug_t group, level_t level, int thread,
	ike_sa_t* ike_sa, const char *message)
{
	if (ike_sa && ike_sa->get_unique_id(ike_sa) == this->ike_sa)
	{
		xpc_object_t msg;

		msg = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(msg, "type", "event");
		xpc_dictionary_set_string(msg, "event", "log");
		xpc_dictionary_set_string(msg, "message", message);
		xpc_connection_send_message(this->conn, msg);
		xpc_release(msg);
	}
}

METHOD(logger_t, get_level, level_t,
	private_xpc_logger_t *this, debug_t group)
{
	return LEVEL_CTRL;
}

METHOD(xpc_logger_t, set_ike_sa, void,
	private_xpc_logger_t *this, uint32_t ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(xpc_logger_t, destroy, void,
	private_xpc_logger_t *this)
{
	free(this);
}

/**
 * See header
 */
xpc_logger_t *xpc_logger_create(xpc_connection_t conn)
{
	private_xpc_logger_t *this;

	INIT(this,
		.public = {
			.logger = {
				.log = _log_,
				.get_level = _get_level,
			},
			.set_ike_sa = _set_ike_sa,
			.destroy = _destroy,
		},
		.conn = conn,
	);

	return &this->public;
}
