/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "xauth_eap.h"

#include <daemon.h>

#include <library.h>
#include <credentials/sets/callback_cred.h>

typedef struct private_xauth_eap_t private_xauth_eap_t;

/**
 * Private data of an xauth_eap_t object.
 */
struct private_xauth_eap_t {

	/**
	 * Public interface.
	 */
	xauth_eap_t public;

	/**
	 * ID of the server
	 */
	identification_t *server;

	/**
	 * ID of the peer
	 */
	identification_t *peer;

	/**
	 * Callback credential set
	 */
	callback_cred_t *cred;

	/**
	 * XAuth password
	 */
	chunk_t pass;
};

/**
 * Callback credential set function
 */
static shared_key_t* shared_cb(private_xauth_eap_t *this, shared_key_type_t type,
							   identification_t *me, identification_t *other,
							   id_match_t *match_me, id_match_t *match_other)
{
	shared_key_t *shared;

	if (!this->pass.len)
	{
		return NULL;
	}
	if (type != SHARED_EAP && type != SHARED_ANY)
	{
		return NULL;
	}
	if (me)
	{
		if (!this->peer->equals(this->peer, me))
		{
			return NULL;
		}
		if (match_me)
		{
			*match_me = ID_MATCH_PERFECT;
		}
	}
	else if (match_me)
	{
		*match_me = ID_MATCH_ANY;
	}
	if (other)
	{
		if (!this->server->equals(this->server, other))
		{
			return NULL;
		}
		if (match_other)
		{
			*match_other = ID_MATCH_PERFECT;
		}
	}
	else if (match_other)
	{
		*match_other = ID_MATCH_ANY;
	}
	shared = shared_key_create(SHARED_EAP, chunk_clone(this->pass));
	this->pass = chunk_empty;
	return shared;
}

/**
 * Do EAP exchanges to verify secret
 */
static bool verify_eap(private_xauth_eap_t *this, eap_method_t *backend)
{
	eap_payload_t *request, *response;
	eap_method_t *frontend;
	eap_type_t type;
	uint32_t vendor;
	status_t status;

	if (backend->initiate(backend, &request) != NEED_MORE)
	{
		return FALSE;
	}
	type = request->get_type(request, &vendor);
	frontend = charon->eap->create_instance(charon->eap, type, vendor,
											EAP_PEER, this->server, this->peer);
	if (!frontend)
	{
		DBG1(DBG_IKE, "XAuth-EAP backend requested %N, but not supported",
			 eap_type_names, type);
		request->destroy(request);
		return FALSE;
	}
	while (TRUE)
	{
		/* credential set is active in frontend only, but not in backend */
		lib->credmgr->add_local_set(lib->credmgr, &this->cred->set, TRUE);
		status = frontend->process(frontend, request, &response);
		lib->credmgr->remove_local_set(lib->credmgr, &this->cred->set);
		request->destroy(request);
		if (status != NEED_MORE)
		{	/* clients should never return SUCCESS */
			frontend->destroy(frontend);
			return FALSE;
		}
		status = backend->process(backend, response, &request);
		response->destroy(response);
		switch (status)
		{
			case SUCCESS:
				frontend->destroy(frontend);
				return TRUE;
			case NEED_MORE:
				break;
			default:
				frontend->destroy(frontend);
				return FALSE;
		}
	}
}

METHOD(xauth_method_t, initiate, status_t,
	private_xauth_eap_t *this, cp_payload_t **out)
{
	cp_payload_t *cp;

	cp = cp_payload_create_type(PLV1_CONFIGURATION, CFG_REQUEST);
	cp->add_attribute(cp, configuration_attribute_create_chunk(
				PLV1_CONFIGURATION_ATTRIBUTE, XAUTH_USER_NAME, chunk_empty));
	cp->add_attribute(cp, configuration_attribute_create_chunk(
				PLV1_CONFIGURATION_ATTRIBUTE, XAUTH_USER_PASSWORD, chunk_empty));
	*out = cp;
	return NEED_MORE;
}

METHOD(xauth_method_t, process, status_t,
	private_xauth_eap_t *this, cp_payload_t *in, cp_payload_t **out)
{
	configuration_attribute_t *attr;
	enumerator_t *enumerator;
	identification_t *id;
	chunk_t user = chunk_empty;
	eap_method_t *backend;
	eap_type_t type;
	char *name;
	bool ok;

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &attr))
	{
		switch (attr->get_type(attr))
		{
			case XAUTH_USER_NAME:
				user = attr->get_chunk(attr);
				break;
			case XAUTH_USER_PASSWORD:
				this->pass = attr->get_chunk(attr);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (!user.ptr || !this->pass.ptr)
	{
		DBG1(DBG_IKE, "peer did not respond to our XAuth request");
		return FAILED;
	}
	if (user.len)
	{
		id = identification_create_from_data(user);
		if (!id)
		{
			DBG1(DBG_IKE, "failed to parse provided XAuth username");
			return FAILED;
		}
		this->peer->destroy(this->peer);
		this->peer = id;
	}
	if (this->pass.len && this->pass.ptr[this->pass.len - 1] == 0)
	{	/* fix null-terminated passwords (Android etc.) */
		this->pass.len -= 1;
	}

	name = lib->settings->get_str(lib->settings,
								  "%s.plugins.xauth-eap.backend", "radius",
								  lib->ns);
	type = eap_type_from_string(name);
	if (!type)
	{
		DBG1(DBG_CFG, "Unknown XAuth-EAP method: %s", name);
		return FAILED;
	}
	backend = charon->eap->create_instance(charon->eap, type, 0, EAP_SERVER,
										   this->server, this->peer);
	if (!backend)
	{
		DBG1(DBG_CFG, "XAuth-EAP method backend not supported: %s", name);
		return FAILED;
	}
	ok = verify_eap(this, backend);
	backend->destroy(backend);
	if (ok)
	{
		return SUCCESS;
	}
	return FAILED;
}

METHOD(xauth_method_t, get_identity, identification_t*,
	private_xauth_eap_t *this)
{
	return this->peer;
}

METHOD(xauth_method_t, destroy, void,
	private_xauth_eap_t *this)
{
	this->cred->destroy(this->cred);
	this->server->destroy(this->server);
	this->peer->destroy(this->peer);
	free(this);
}

/*
 * Described in header.
 */
xauth_eap_t *xauth_eap_create_server(identification_t *server,
									 identification_t *peer, char *profile)
{
	private_xauth_eap_t *this;

	INIT(this,
		.public = {
			.xauth_method = {
				.initiate = _initiate,
				.process = _process,
				.get_identity = _get_identity,
				.destroy = _destroy,
			},
		},
		.server = server->clone(server),
		.peer = peer->clone(peer),
	);

	this->cred = callback_cred_create_shared((void*)shared_cb, this);

	return &this->public;
}
