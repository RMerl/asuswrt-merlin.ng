/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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

#include "eap_peap.h"
#include "eap_peap_peer.h"
#include "eap_peap_server.h"

#include <tls_eap.h>

#include <daemon.h>
#include <library.h>

typedef struct private_eap_peap_t private_eap_peap_t;

/**
 * Private data of an eap_peap_t object.
 */
struct private_eap_peap_t {

	/**
	 * Public interface.
	 */
	eap_peap_t public;

	/**
	 * TLS stack, wrapped by EAP helper
	 */
	tls_eap_t *tls_eap;
};

/** Maximum number of EAP-PEAP messages/fragments allowed */
#define MAX_MESSAGE_COUNT 32
/** Default size of a EAP-PEAP fragment */
#define MAX_FRAGMENT_LEN 1024

METHOD(eap_method_t, initiate, status_t,
	private_eap_peap_t *this, eap_payload_t **out)
{
	chunk_t data;

	if (this->tls_eap->initiate(this->tls_eap, &data) == NEED_MORE)
	{
		*out = eap_payload_create_data(data);
		free(data.ptr);
		return NEED_MORE;
	}
	return FAILED;
}

METHOD(eap_method_t, process, status_t,
	private_eap_peap_t *this, eap_payload_t *in, eap_payload_t **out)
{
	status_t status;
	chunk_t data;

	data = in->get_data(in);
	status = this->tls_eap->process(this->tls_eap, data, &data);
	if (status == NEED_MORE)
	{
		*out = eap_payload_create_data(data);
		free(data.ptr);
	}
	return status;
}

METHOD(eap_method_t, get_type, eap_type_t,
	private_eap_peap_t *this, pen_t *vendor)
{
	*vendor = 0;
	return EAP_PEAP;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_peap_t *this, chunk_t *msk)
{
	*msk = this->tls_eap->get_msk(this->tls_eap);
	if (msk->len)
	{
		return SUCCESS;
	}
	return FAILED;
}

METHOD(eap_method_t, get_identifier, uint8_t,
	private_eap_peap_t *this)
{
	return this->tls_eap->get_identifier(this->tls_eap);
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_peap_t *this, uint8_t identifier)
{
	this->tls_eap->set_identifier(this->tls_eap, identifier);
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_peap_t *this)
{
	return TRUE;
}

METHOD(eap_method_t, destroy, void,
	private_eap_peap_t *this)
{
	this->tls_eap->destroy(this->tls_eap);
	free(this);
}

/**
 * Create an empty private eap_peap_t object
 */
static private_eap_peap_t *eap_peap_create_empty(void)
{
	private_eap_peap_t *this;

	INIT(this,
		.public = {
			.eap_method = {
				.initiate = _initiate,
				.process = _process,
				.get_type = _get_type,
				.is_mutual = _is_mutual,
				.get_msk = _get_msk,
				.get_identifier = _get_identifier,
				.set_identifier = _set_identifier,
				.destroy = _destroy,
			},
		},
	);
	return this;
}

/**
 * Generic private constructor
 */
static eap_peap_t *eap_peap_create(private_eap_peap_t * this,
								   identification_t *server,
								   identification_t *peer, bool is_server,
								   tls_application_t *application)
{
	size_t frag_size;
	int max_msg_count;
	bool include_length;
	tls_t *tls;

	if (is_server && !lib->settings->get_bool(lib->settings,
								"%s.plugins.eap-peap.request_peer_auth", FALSE,
								lib->ns))
	{
		peer = NULL;
	}
	frag_size = lib->settings->get_int(lib->settings,
					"%s.plugins.eap-peap.fragment_size", MAX_FRAGMENT_LEN,
					lib->ns);
	max_msg_count = lib->settings->get_int(lib->settings,
					"%s.plugins.eap-peap.max_message_count", MAX_MESSAGE_COUNT,
					lib->ns);
	include_length = lib->settings->get_bool(lib->settings,
					"%s.plugins.eap-peap.include_length", FALSE, lib->ns);
	tls = tls_create(is_server, server, peer, TLS_PURPOSE_EAP_PEAP,
					 application, NULL, 0);
	this->tls_eap = tls_eap_create(EAP_PEAP, tls, frag_size, max_msg_count,
												  include_length);
	if (!this->tls_eap)
	{
		application->destroy(application);
		free(this);
		return NULL;
	}
	return &this->public;
}

eap_peap_t *eap_peap_create_server(identification_t *server,
								   identification_t *peer)
{
	private_eap_peap_t *eap_peap;
	eap_method_t *eap_method;
	eap_peap_server_t *eap_peap_server;
	tls_application_t *application;

	/* the tunneled application needs a reference to the outer EAP-PEAP method */
	eap_peap = eap_peap_create_empty();
	eap_method = &eap_peap->public.eap_method;
	eap_peap_server = eap_peap_server_create(server, peer, eap_method);
	application = &eap_peap_server->application;

	return eap_peap_create(eap_peap, server, peer, TRUE, application);
}

eap_peap_t *eap_peap_create_peer(identification_t *server,
								 identification_t *peer)
{
	private_eap_peap_t *eap_peap;
	eap_method_t *eap_method;
	eap_peap_peer_t *eap_peap_peer;
	tls_application_t *application;

	/* the tunneled application needs a reference to the outer EAP-PEAP method */
	eap_peap = eap_peap_create_empty();
	eap_method = &eap_peap->public.eap_method;
	eap_peap_peer = eap_peap_peer_create(server, peer, eap_method);
	application = &eap_peap_peer->application;

	return eap_peap_create(eap_peap, server, peer, FALSE, application);
}
