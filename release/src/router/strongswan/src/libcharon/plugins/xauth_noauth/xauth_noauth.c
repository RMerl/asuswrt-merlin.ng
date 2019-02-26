/*
 * Copyright (C) 2013 Tobias Brunner
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

#include "xauth_noauth.h"

#include <daemon.h>
#include <library.h>

typedef struct private_xauth_noauth_t private_xauth_noauth_t;

/**
 * Private data of an xauth_noauth_t object.
 */
struct private_xauth_noauth_t {

	/**
	 * Public interface.
	 */
	xauth_noauth_t public;

	/**
	 * ID of the peer (not really used here)
	 */
	identification_t *peer;

};

METHOD(xauth_method_t, initiate, status_t,
	private_xauth_noauth_t *this, cp_payload_t **out)
{
	/* XAuth task handles the details for us */
	return SUCCESS;
}

METHOD(xauth_method_t, process, status_t,
	private_xauth_noauth_t *this, cp_payload_t *in, cp_payload_t **out)
{
	/* this should never be called */
	return FAILED;
}

METHOD(xauth_method_t, get_identity, identification_t*,
	private_xauth_noauth_t *this)
{
	/* this should never be called, but lets still return a valid ID */
	return this->peer;
}

METHOD(xauth_method_t, destroy, void,
	private_xauth_noauth_t *this)
{
	this->peer->destroy(this->peer);
	free(this);
}

/*
 * Described in header.
 */
xauth_noauth_t *xauth_noauth_create_server(identification_t *server,
										   identification_t *peer,
										   char *profile)
{
	private_xauth_noauth_t *this;

	INIT(this,
		.public = {
			.xauth_method = {
				.initiate = _initiate,
				.process = _process,
				.get_identity = _get_identity,
				.destroy = _destroy,
			},
		},
		.peer = identification_create_from_string("%any"),
	);

	return &this->public;
}
