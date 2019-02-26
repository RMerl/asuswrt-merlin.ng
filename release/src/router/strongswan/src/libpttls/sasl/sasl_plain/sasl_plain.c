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

#include "sasl_plain.h"

#include <utils/debug.h>

typedef struct private_sasl_plain_t private_sasl_plain_t;

/**
 * Private data of an sasl_plain_t object.
 */
struct private_sasl_plain_t {

	/**
	 * Public sasl_plain_t interface.
	 */
	sasl_plain_t public;

	/**
	 * Client identity
	 */
	identification_t *client;
};

METHOD(sasl_mechanism_t, get_client, identification_t*,
	private_sasl_plain_t *this)
{
	return this->client;
}

METHOD(sasl_mechanism_t, get_name, char*,
	private_sasl_plain_t *this)
{
	return "PLAIN";
}

METHOD(sasl_mechanism_t, build_server, status_t,
	private_sasl_plain_t *this, chunk_t *message)
{
	/* gets never called */
	return FAILED;
}

METHOD(sasl_mechanism_t, process_server, status_t,
	private_sasl_plain_t *this, chunk_t message)
{
	chunk_t authz, authi, password;
	shared_key_t *shared;
	u_char *pos;

	pos = memchr(message.ptr, 0, message.len);
	if (!pos)
	{
		DBG1(DBG_CFG, "invalid authz encoding");
		return FAILED;
	}
	authz = chunk_create(message.ptr, pos - message.ptr);
	message = chunk_skip(message, authz.len + 1);
	pos = memchr(message.ptr, 0, message.len);
	if (!pos)
	{
		DBG1(DBG_CFG, "invalid authi encoding");
		return FAILED;
	}
	authi = chunk_create(message.ptr, pos - message.ptr);
	password = chunk_skip(message, authi.len + 1);
	DESTROY_IF(this->client);
	this->client = identification_create_from_data(authi);
	shared = lib->credmgr->get_shared(lib->credmgr, SHARED_EAP, this->client,
									  NULL);
	if (!shared)
	{
		DBG1(DBG_CFG, "no shared secret found for '%Y'", this->client);
		return FAILED;
	}
	if (!chunk_equals_const(shared->get_key(shared), password))
	{
		DBG1(DBG_CFG, "shared secret for '%Y' does not match", this->client);
		shared->destroy(shared);
		return FAILED;
	}
	shared->destroy(shared);
	return SUCCESS;
}

METHOD(sasl_mechanism_t, build_client, status_t,
	private_sasl_plain_t *this, chunk_t *message)
{
	shared_key_t *shared;
	chunk_t password;
	char buf[256];
	ssize_t len;

	/* we currently use the EAP type of shared secret */
	shared = lib->credmgr->get_shared(lib->credmgr, SHARED_EAP,
									  this->client, NULL);
	if (!shared)
	{
		DBG1(DBG_CFG, "no shared secret found for %Y", this->client);
		return FAILED;
	}

	password = shared->get_key(shared);
	len = snprintf(buf, sizeof(buf), "%s%c%Y%c%.*s",
				   "", 0, this->client, 0,
				   (int)password.len, password.ptr);
	shared->destroy(shared);

	if (len < 0 || len >= sizeof(buf))
	{
		return FAILED;
	}
	*message = chunk_clone(chunk_create(buf, len));

	return NEED_MORE;
}

METHOD(sasl_mechanism_t, process_client, status_t,
	private_sasl_plain_t *this, chunk_t message)
{
	/* if the server sends a result, authentication successful */
	return SUCCESS;
}

METHOD(sasl_mechanism_t, destroy, void,
	private_sasl_plain_t *this)
{
	DESTROY_IF(this->client);
	free(this);
}

/**
 * See header
 */
sasl_plain_t *sasl_plain_create(char *name, identification_t *client)
{
	private_sasl_plain_t *this;

	if (!streq(get_name(NULL), name))
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.sasl = {
				.get_name = _get_name,
				.get_client = _get_client,
				.destroy = _destroy,
			},
		},
	);

	if (client)
	{
		this->public.sasl.build = _build_client;
		this->public.sasl.process = _process_client;
		this->client = client->clone(client);
	}
	else
	{
		this->public.sasl.build = _build_server;
		this->public.sasl.process = _process_server;
	}
	return &this->public;
}
