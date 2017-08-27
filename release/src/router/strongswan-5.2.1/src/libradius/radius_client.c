/*
 * Copyright (C) 2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include "radius_client.h"
#include "radius_config.h"

#include <unistd.h>
#include <errno.h>

#include <utils/debug.h>
#include <networking/host.h>
#include <collections/linked_list.h>
#include <threading/condvar.h>
#include <threading/mutex.h>

typedef struct private_radius_client_t private_radius_client_t;

/**
 * Private data of an radius_client_t object.
 */
struct private_radius_client_t {

	/**
	 * Public radius_client_t interface.
	 */
	radius_client_t public;

	/**
	 * Selected RADIUS server configuration
	 */
	radius_config_t *config;

	/**
	 * RADIUS servers State attribute
	 */
	chunk_t state;

	/**
	 * EAP MSK, from MPPE keys
	 */
	chunk_t msk;
};

/**
 * Save the state attribute to include in further request
 */
static void save_state(private_radius_client_t *this, radius_message_t *msg)
{
	enumerator_t *enumerator;
	int type;
	chunk_t data;

	enumerator = msg->create_enumerator(msg);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		if (type == RAT_STATE)
		{
			free(this->state.ptr);
			this->state = chunk_clone(data);
			enumerator->destroy(enumerator);
			return;
		}
	}
	enumerator->destroy(enumerator);
	/* no state attribute found, remove state */
	chunk_free(&this->state);
}

METHOD(radius_client_t, request, radius_message_t*,
	private_radius_client_t *this, radius_message_t *req)
{
	radius_socket_t *socket;
	radius_message_t *res;
	chunk_t data;

	/* add our NAS-Identifier */
	req->add(req, RAT_NAS_IDENTIFIER,
			 this->config->get_nas_identifier(this->config));
	/* add State attribute, if server sent one */
	if (this->state.ptr)
	{
		req->add(req, RAT_STATE, this->state);
	}
	socket = this->config->get_socket(this->config);
	DBG1(DBG_CFG, "sending RADIUS %N to server '%s'", radius_message_code_names,
		 req->get_code(req), this->config->get_name(this->config));

	res = socket->request(socket, req);
	if (res)
	{
		DBG1(DBG_CFG, "received RADIUS %N from server '%s'",
			 radius_message_code_names, res->get_code(res),
			 this->config->get_name(this->config));
		data = res->get_encoding(res);
		DBG3(DBG_CFG, "%B", &data);

		save_state(this, res);
		if (res->get_code(res) == RMC_ACCESS_ACCEPT)
		{
			chunk_clear(&this->msk);
			this->msk = socket->decrypt_msk(socket, req, res);
		}
		this->config->put_socket(this->config, socket, TRUE);
		return res;
	}
	this->config->put_socket(this->config, socket, FALSE);
	return NULL;
}

METHOD(radius_client_t, get_msk, chunk_t,
	private_radius_client_t *this)
{
	return this->msk;
}

METHOD(radius_client_t, destroy, void,
	private_radius_client_t *this)
{
	this->config->destroy(this->config);
	chunk_clear(&this->msk);
	free(this->state.ptr);
	free(this);
}

/**
 * See header
 */
radius_client_t *radius_client_create(radius_config_t *config)
{
	private_radius_client_t *this;

	INIT(this,
		.public = {
			.request = _request,
			.get_msk = _get_msk,
			.destroy = _destroy,
		},
		.config = config,
	);

	return &this->public;
}
