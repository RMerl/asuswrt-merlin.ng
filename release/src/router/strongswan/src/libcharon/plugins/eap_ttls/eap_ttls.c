/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 *
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

#include "eap_ttls.h"
#include "eap_ttls_peer.h"
#include "eap_ttls_server.h"

#include <tls_eap.h>

#include <daemon.h>
#include <library.h>

typedef struct private_eap_ttls_t private_eap_ttls_t;

/**
 * Private data of an eap_ttls_t object.
 */
struct private_eap_ttls_t {

	/**
	 * Public interface.
	 */
	eap_ttls_t public;

	/**
	 * TLS stack, wrapped by EAP helper
	 */
	tls_eap_t *tls_eap;
};

/** Maximum number of EAP-TTLS messages/fragments allowed */
#define MAX_MESSAGE_COUNT 32
/** Default size of a EAP-TTLS fragment */
#define MAX_FRAGMENT_LEN 1024

METHOD(eap_method_t, initiate, status_t,
	private_eap_ttls_t *this, eap_payload_t **out)
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
	private_eap_ttls_t *this, eap_payload_t *in, eap_payload_t **out)
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
	private_eap_ttls_t *this, uint32_t *vendor)
{
	*vendor = 0;
	return EAP_TTLS;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_ttls_t *this, chunk_t *msk)
{
	*msk = this->tls_eap->get_msk(this->tls_eap);
	if (msk->len)
	{
		return SUCCESS;
	}
	return FAILED;
}

METHOD(eap_method_t, get_identifier, uint8_t,
	private_eap_ttls_t *this)
{
	return this->tls_eap->get_identifier(this->tls_eap);
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_ttls_t *this, uint8_t identifier)
{
	this->tls_eap->set_identifier(this->tls_eap, identifier);
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_ttls_t *this)
{
	return TRUE;
}

METHOD(eap_method_t, get_auth, auth_cfg_t*,
	private_eap_ttls_t *this)
{
	return this->tls_eap->get_auth(this->tls_eap);
}

METHOD(eap_method_t, destroy, void,
	private_eap_ttls_t *this)
{
	this->tls_eap->destroy(this->tls_eap);
	free(this);
}

/**
 * Generic private constructor
 */
static eap_ttls_t *eap_ttls_create(identification_t *server,
								   identification_t *peer, bool is_server,
								   tls_application_t *application)
{
	private_eap_ttls_t *this;
	size_t frag_size;
	int max_msg_count;
	bool include_length;
	tls_t *tls;

	INIT(this,
		.public = {
			.eap_method = {
				.initiate = _initiate,
				.process = _process,
				.get_type = _get_type,
				.is_mutual = _is_mutual,
				.get_identifier = _get_identifier,
				.set_identifier = _set_identifier,
				.get_msk = _get_msk,
				.get_auth = _get_auth,
				.destroy = _destroy,
			},
		},
	);
	if (is_server && !lib->settings->get_bool(lib->settings,
								"%s.plugins.eap-ttls.request_peer_auth", FALSE,
								lib->ns))
	{
		peer = NULL;
	}
	frag_size = lib->settings->get_int(lib->settings,
					"%s.plugins.eap-ttls.fragment_size", MAX_FRAGMENT_LEN,
					lib->ns);
	max_msg_count = lib->settings->get_int(lib->settings,
					"%s.plugins.eap-ttls.max_message_count", MAX_MESSAGE_COUNT,
					lib->ns);
	include_length = lib->settings->get_bool(lib->settings,
					"%s.plugins.eap-ttls.include_length", TRUE, lib->ns);
	tls = tls_create(is_server, server, peer, TLS_PURPOSE_EAP_TTLS,
					 application, NULL);
	this->tls_eap = tls_eap_create(EAP_TTLS, tls, frag_size, max_msg_count,
												  include_length);
	if (!this->tls_eap)
	{
		application->destroy(application);
		free(this);
		return NULL;
	}
	return &this->public;
}

eap_ttls_t *eap_ttls_create_server(identification_t *server,
								   identification_t *peer)
{
	return eap_ttls_create(server, peer, TRUE,
						   &eap_ttls_server_create(server, peer)->application);
}

eap_ttls_t *eap_ttls_create_peer(identification_t *server,
								 identification_t *peer)
{
	return eap_ttls_create(server, peer, FALSE,
						   &eap_ttls_peer_create(server, peer)->application);
}
