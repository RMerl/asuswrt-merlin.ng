/*
 * Copyright (C) 2011-2015 Andreas Steffen
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

#include "tnccs_dynamic.h"

#include <tnc/tnc.h>

#include <utils/debug.h>

typedef struct private_tnccs_dynamic_t private_tnccs_dynamic_t;

/**
 * Private data of a tnccs_dynamic_t object.
 */
struct private_tnccs_dynamic_t {

	/**
	 * Public tnccs_t interface.
	 */
	tnccs_t public;

	/**
	 * Server identity
	 */
	identification_t *server_id;

	/**
	 * Client identity
	 */
	identification_t *peer_id;

	/**
	 * Server IP address
	 */
	host_t *server_ip;

	/**
	 * Client IP address
	 */
	host_t *peer_ip;

	/**
	 * Detected TNC IF-TNCCS stack
	 */
	tls_t *tls;

	/**
	 * Underlying TNC IF-T transport protocol
	 */
	tnc_ift_type_t transport;

	/**
	 * Type of TNC client authentication
	 */
	uint32_t auth_type;

	/**
	 * Callback function to communicate recommendation (TNC Server only)
	 */
	tnccs_cb_t callback;

	/**
	 * reference count
	 */
	refcount_t ref;

};

/**
 * Determine the version of the IF-TNCCS protocol used by analyzing the first
 * byte of the TNCCS batch received from a TNC Client according to the rules
 * defined by section 3.5 "Interoperability with older IF-TNCCS versions" of
 * the TCG TNC IF-TNCCS TLV Bindings Version 2.0 standard.
 */
tnccs_type_t determine_tnccs_protocol(char version)
{
	switch (version)
	{
		case '\t':
		case '\n':
		case '\r':
		case ' ':
		case '<':
			return TNCCS_1_1;
		case 0x00:
			return TNCCS_SOH;
		case 0x02:
			return TNCCS_2_0;
		default:
			return TNCCS_UNKNOWN;
	}
}

METHOD(tls_t, process, status_t,
	private_tnccs_dynamic_t *this, void *buf, size_t buflen)
{
	tnccs_type_t type;
	tnccs_t *tnccs;

	if (!this->tls)
	{
		if (buflen == 0)
		{
			return FAILED;
		}
		type = determine_tnccs_protocol(*(char*)buf);
		DBG1(DBG_TNC, "%N protocol detected dynamically",
					   tnccs_type_names, type);
		tnccs = tnc->tnccs->create_instance(tnc->tnccs, type, TRUE,
							this->server_id, this->peer_id, this->server_ip,
							this->peer_ip, this->transport,	this->callback);
		if (!tnccs)
		{
			DBG1(DBG_TNC, "N% protocol not supported", tnccs_type_names, type);
			return FAILED;
		}
		tnccs->set_auth_type(tnccs, this->auth_type);
		this->tls = &tnccs->tls;
	}
	return this->tls->process(this->tls, buf, buflen);
}

METHOD(tls_t, build, status_t,
	private_tnccs_dynamic_t *this, void *buf, size_t *buflen, size_t *msglen)
{
	return this->tls->build(this->tls, buf, buflen, msglen);
}

METHOD(tls_t, is_server, bool,
	private_tnccs_dynamic_t *this)
{
	return TRUE;
}

METHOD(tls_t, get_server_id, identification_t*,
	private_tnccs_dynamic_t *this)
{
	return this->server_id;
}

METHOD(tls_t, set_peer_id, void,
	private_tnccs_dynamic_t *this, identification_t *id)
{
	DESTROY_IF(this->peer_id);
	this->peer_id = id->clone(id);
	if (this->tls)
	{
		this->tls->set_peer_id(this->tls, id);
	}
}

METHOD(tls_t, get_peer_id, identification_t*,
	private_tnccs_dynamic_t *this)
{
	return this->peer_id;
}

METHOD(tls_t, get_purpose, tls_purpose_t,
	private_tnccs_dynamic_t *this)
{
	return TLS_PURPOSE_EAP_TNC;
}

METHOD(tls_t, is_complete, bool,
	private_tnccs_dynamic_t *this)
{
	return this->tls ? this->tls->is_complete(this->tls) : FALSE;
}

METHOD(tls_t, get_eap_msk, chunk_t,
	private_tnccs_dynamic_t *this)
{
	return chunk_empty;
}

METHOD(tls_t, destroy, void,
	private_tnccs_dynamic_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->tls);
		this->server_id->destroy(this->server_id);
		this->peer_id->destroy(this->peer_id);
		this->server_ip->destroy(this->server_ip);
		this->peer_ip->destroy(this->peer_ip);
		free(this);
	}
}

METHOD(tnccs_t, get_server_ip, host_t*,
	private_tnccs_dynamic_t *this)
{
	return this->server_ip;
}

METHOD(tnccs_t, get_peer_ip, host_t*,
	private_tnccs_dynamic_t *this)
{
	return this->peer_ip;
}

METHOD(tnccs_t, get_transport, tnc_ift_type_t,
	private_tnccs_dynamic_t *this)
{
	return this->transport;
}

METHOD(tnccs_t, set_transport, void,
	private_tnccs_dynamic_t *this, tnc_ift_type_t transport)
{
	this->transport = transport;
}

METHOD(tnccs_t, get_auth_type, uint32_t,
	private_tnccs_dynamic_t *this)
{
	return this->auth_type;
}

METHOD(tnccs_t, set_auth_type, void,
	private_tnccs_dynamic_t *this, uint32_t auth_type)
{
	this->auth_type = auth_type;
}

METHOD(tnccs_t, get_pdp_server, chunk_t,
	private_tnccs_dynamic_t *this, uint16_t *port)
{
	tnccs_t *tnccs = (tnccs_t*)this->tls;

	return tnccs->get_pdp_server(tnccs, port);
}

METHOD(tnccs_t, get_ref, tnccs_t*,
	private_tnccs_dynamic_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

/**
 * See header
 */
tnccs_t* tnccs_dynamic_create(bool is_server, identification_t *server_id,
							  identification_t *peer_id, host_t *server_ip,
							  host_t *peer_ip, tnc_ift_type_t transport,
							  tnccs_cb_t cb)
{
	private_tnccs_dynamic_t *this;

	INIT(this,
		.public = {
			.tls = {
				.process = _process,
				.build = _build,
				.is_server = _is_server,
				.get_server_id = _get_server_id,
				.set_peer_id = _set_peer_id,
				.get_peer_id = _get_peer_id,
				.get_purpose = _get_purpose,
				.is_complete = _is_complete,
				.get_eap_msk = _get_eap_msk,
				.destroy = _destroy,
			},
			.get_server_ip = _get_server_ip,
			.get_peer_ip = _get_peer_ip,
			.get_transport = _get_transport,
			.set_transport = _set_transport,
			.get_auth_type = _get_auth_type,
			.set_auth_type = _set_auth_type,
			.get_pdp_server = _get_pdp_server,
			.get_ref = _get_ref,
		},
		.server_id = server_id->clone(server_id),
		.peer_id = peer_id->clone(peer_id),
		.server_ip = server_ip->clone(server_ip),
		.peer_ip = peer_ip->clone(peer_ip),
		.transport = transport,
		.callback = cb,
		.ref = 1,
	);

	return &this->public;
}
