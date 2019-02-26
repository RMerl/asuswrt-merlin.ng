/*
 * Copyright (C) 2007-2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "eap_gtc.h"

#include <daemon.h>
#include <library.h>

#define GTC_REQUEST_MSG "password"

typedef struct private_eap_gtc_t private_eap_gtc_t;

/**
 * Private data of an eap_gtc_t object.
 */
struct private_eap_gtc_t {

	/**
	 * Public authenticator_t interface.
	 */
	eap_gtc_t public;

	/**
	 * ID of the server
	 */
	identification_t *server;

	/**
	 * ID of the peer
	 */
	identification_t *peer;

	/**
	 * EAP message identififier
	 */
	uint8_t identifier;
};

typedef struct eap_gtc_header_t eap_gtc_header_t;

/**
 * packed eap GTC header struct
 */
struct eap_gtc_header_t {
	/** EAP code (REQUEST/RESPONSE) */
	uint8_t code;
	/** unique message identifier */
	uint8_t identifier;
	/** length of whole message */
	uint16_t length;
	/** EAP type */
	uint8_t type;
	/** type data */
	uint8_t data[];
} __attribute__((__packed__));

METHOD(eap_method_t, initiate_peer, status_t,
	private_eap_gtc_t *this, eap_payload_t **out)
{
	/* peer never initiates */
	return FAILED;
}

METHOD(eap_method_t, initiate_server, status_t,
	private_eap_gtc_t *this, eap_payload_t **out)
{
	eap_gtc_header_t *req;
	size_t len;

	len = strlen(GTC_REQUEST_MSG);
	req = alloca(sizeof(eap_gtc_header_t) + len);
	req->length = htons(sizeof(eap_gtc_header_t) + len);
	req->code = EAP_REQUEST;
	req->identifier = this->identifier;
	req->type = EAP_GTC;
	memcpy(req->data, GTC_REQUEST_MSG, len);

	*out = eap_payload_create_data(chunk_create((void*)req,
								   sizeof(eap_gtc_header_t) + len));
	return NEED_MORE;
}

METHOD(eap_method_t, process_peer, status_t,
	private_eap_gtc_t *this, eap_payload_t *in, eap_payload_t **out)
{
	eap_gtc_header_t *res;
	shared_key_t *shared;
	chunk_t key;
	size_t len;

	shared = lib->credmgr->get_shared(lib->credmgr, SHARED_EAP,
									  this->peer, this->server);
	if (shared == NULL)
	{
		DBG1(DBG_IKE, "no EAP key found for '%Y' - '%Y'",
			 this->peer, this->server);
		return FAILED;
	}
	key = shared->get_key(shared);
	len = key.len;

	/* TODO: According to the draft we should "SASLprep" password, RFC4013. */

	this->identifier = in->get_identifier(in);
	res = alloca(sizeof(eap_gtc_header_t) + len);
	res->length = htons(sizeof(eap_gtc_header_t) + len);
	res->code = EAP_RESPONSE;
	res->identifier = this->identifier;
	res->type = EAP_GTC;
	memcpy(res->data, key.ptr, len);

	shared->destroy(shared);

	*out = eap_payload_create_data(chunk_create((void*)res,
								   sizeof(eap_gtc_header_t) + len));
	return NEED_MORE;
}

METHOD(eap_method_t, process_server, status_t,
	private_eap_gtc_t *this, eap_payload_t *in, eap_payload_t **out)
{
	status_t status = FAILED;
	chunk_t user, pass;
	xauth_method_t *xauth;
	cp_payload_t *ci, *co;
	char *backend;

	user = this->peer->get_encoding(this->peer);
	pass = chunk_skip(in->get_data(in), 5);
	if (this->identifier != in->get_identifier(in) || !pass.len)
	{
		DBG1(DBG_IKE, "received invalid EAP-GTC message");
		return FAILED;
	}

	/* get XAuth backend to use for credential verification. Default to PAM
	 * to support legacy EAP-GTC configurations */
	backend = lib->settings->get_str(lib->settings,
								"%s.plugins.eap-gtc.backend", "pam", lib->ns);
	xauth = charon->xauth->create_instance(charon->xauth, backend, XAUTH_SERVER,
										   this->server, this->peer);
	if (!xauth)
	{
		DBG1(DBG_IKE, "creating EAP-GTC XAuth backend '%s' failed", backend);
		return FAILED;
	}
	if (xauth->initiate(xauth, &co) == NEED_MORE)
	{
		/* assume that "out" contains username/password attributes */
		co->destroy(co);
		ci = cp_payload_create_type(PLV1_CONFIGURATION, CFG_REPLY);
		ci->add_attribute(ci, configuration_attribute_create_chunk(
					PLV1_CONFIGURATION_ATTRIBUTE, XAUTH_USER_NAME, user));
		ci->add_attribute(ci, configuration_attribute_create_chunk(
					PLV1_CONFIGURATION_ATTRIBUTE, XAUTH_USER_PASSWORD, pass));
		switch (xauth->process(xauth, ci, &co))
		{
			case SUCCESS:
				status = SUCCESS;
				break;
			case NEED_MORE:
				/* TODO: multiple exchanges currently not supported */
				co->destroy(co);
				break;
			case FAILED:
			default:
				break;
		}
		ci->destroy(ci);
	}
	xauth->destroy(xauth);
	return status;
}

METHOD(eap_method_t, get_type, eap_type_t,
	private_eap_gtc_t *this, uint32_t *vendor)
{
	*vendor = 0;
	return EAP_GTC;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_gtc_t *this, chunk_t *msk)
{
	return FAILED;
}

METHOD(eap_method_t, get_identifier, uint8_t,
	private_eap_gtc_t *this)
{
	return this->identifier;
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_gtc_t *this, uint8_t identifier)
{
	this->identifier = identifier;
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_gtc_t *this)
{
	return FALSE;
}

METHOD(eap_method_t, destroy, void,
	private_eap_gtc_t *this)
{
	this->peer->destroy(this->peer);
	this->server->destroy(this->server);
	free(this);
}

/**
 * Generic constructor
 */
static private_eap_gtc_t *eap_gtc_create_generic(identification_t *server,
												 identification_t *peer)
{
	private_eap_gtc_t *this;

	INIT(this,
		.public = {
			.eap_method_interface = {
				.get_type = _get_type,
				.is_mutual = _is_mutual,
				.get_msk = _get_msk,
				.get_identifier = _get_identifier,
				.set_identifier = _set_identifier,
				.destroy = _destroy,
			},
		},
		.peer = peer->clone(peer),
		.server = server->clone(server),
	);

	return this;
}

/*
 * see header
 */
eap_gtc_t *eap_gtc_create_server(identification_t *server, identification_t *peer)
{
	private_eap_gtc_t *this = eap_gtc_create_generic(server, peer);

	this->public.eap_method_interface.initiate = _initiate_server;
	this->public.eap_method_interface.process = _process_server;

	/* generate a non-zero identifier */
	do {
		this->identifier = random();
	} while (!this->identifier);

	return &this->public;
}

/*
 * see header
 */
eap_gtc_t *eap_gtc_create_peer(identification_t *server, identification_t *peer)
{
	private_eap_gtc_t *this = eap_gtc_create_generic(server, peer);

	this->public.eap_method_interface.initiate = _initiate_peer;
	this->public.eap_method_interface.process = _process_peer;

	return &this->public;
}
