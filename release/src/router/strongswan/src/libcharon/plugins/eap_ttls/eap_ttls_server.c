/*
 * Copyright (C) 2010-2014 Andreas Steffen
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

#include "eap_ttls_server.h"
#include "eap_ttls_avp.h"

#include <utils/debug.h>
#include <daemon.h>

#include <sa/eap/eap_method.h>
#include <sa/eap/eap_inner_method.h>

typedef struct private_eap_ttls_server_t private_eap_ttls_server_t;

/**
 * Private data of an eap_ttls_server_t object.
 */
struct private_eap_ttls_server_t {

	/**
	 * Public eap_ttls_server_t interface.
	 */
	eap_ttls_server_t public;

	/**
	 * Server identity
	 */
	identification_t *server;

	/**
	 * Peer identity
	 */
	identification_t *peer;

	/**
	 * Current EAP-TTLS phase2 state
	 */
	bool start_phase2;

	/**
	 * Current EAP-TTLS phase2 TNC state
	 */
	bool start_phase2_tnc;

	/**
     * Current phase 2 EAP method
	 */
	eap_method_t *method;

	/**
     * Pending outbound EAP message
	 */
	eap_payload_t *out;

	/**
	 * AVP handler
	 */
	eap_ttls_avp_t *avp;
};

/**
 * Start EAP client authentication protocol
 */
static status_t start_phase2_auth(private_eap_ttls_server_t *this)
{
	char *eap_type_str;
	eap_type_t type;

	eap_type_str = lib->settings->get_str(lib->settings,
									"%s.plugins.eap-ttls.phase2_method", "md5",
									lib->ns);
	type = eap_type_from_string(eap_type_str);
	if (type == 0)
	{
		DBG1(DBG_IKE, "unrecognized phase2 method \"%s\"", eap_type_str);
		return FAILED;
	}
	DBG1(DBG_IKE, "phase2 method %N selected", eap_type_names, type);
		this->method = charon->eap->create_instance(charon->eap, type, 0,
								EAP_SERVER, this->server, this->peer);
	if (this->method == NULL)
	{
		DBG1(DBG_IKE, "%N method not available", eap_type_names, type);
		return FAILED;
	}
	if (this->method->initiate(this->method, &this->out) == NEED_MORE)
	{
		return NEED_MORE;
	}
	else
	{
		DBG1(DBG_IKE, "%N method failed", eap_type_names, type);
			return FAILED;
	}
}

/**
 * If configured, start PT-EAP or legacy EAP-TNC protocol
 */
static status_t start_phase2_tnc(private_eap_ttls_server_t *this,
								 eap_type_t auth_type)
{
	eap_inner_method_t *inner_method;
	eap_type_t type;
	char *eap_type_str;

	if (this->start_phase2_tnc && lib->settings->get_bool(lib->settings,
							"%s.plugins.eap-ttls.phase2_tnc", FALSE, lib->ns))
	{
		eap_type_str = lib->settings->get_str(lib->settings,
							"%s.plugins.eap-ttls.phase2_tnc_method", "pt",
							lib->ns);
		type = eap_type_from_string(eap_type_str);
		if (type == 0)
		{
			DBG1(DBG_IKE, "unrecognized phase2 EAP TNC method \"%s\"",
						   eap_type_str);
			return FAILED;
		}
		DBG1(DBG_IKE, "phase2 method %N selected", eap_type_names, type);
		this->method = charon->eap->create_instance(charon->eap, type,
									0, EAP_SERVER, this->server, this->peer);
		if (this->method == NULL)
		{
			DBG1(DBG_IKE, "%N method not available", eap_type_names, type);
			return FAILED;
		}
		inner_method = (eap_inner_method_t *)this->method;
		inner_method->set_auth_type(inner_method, auth_type);

		this->start_phase2_tnc = FALSE;
		if (this->method->initiate(this->method, &this->out) == NEED_MORE)
		{
			return NEED_MORE;
		}
		else
		{
			DBG1(DBG_IKE, "%N method failed", eap_type_names, type);
			return FAILED;
		}
	}
	return SUCCESS;
}

METHOD(tls_application_t, process, status_t,
	private_eap_ttls_server_t *this, bio_reader_t *reader)
{
	chunk_t data = chunk_empty;
	status_t status;
	payload_t *payload;
	eap_payload_t *in;
	eap_code_t code;
	eap_type_t type = EAP_NAK, received_type;
	uint32_t vendor, received_vendor;

	status = this->avp->process(this->avp, reader, &data);
	switch (status)
	{
		case SUCCESS:
			break;
		case NEED_MORE:
			return NEED_MORE;
		case FAILED:
		default:
			return FAILED;
	}
	in = eap_payload_create_data(data);
	chunk_free(&data);
	payload = (payload_t*)in;

	if (payload->verify(payload) != SUCCESS)
	{
		in->destroy(in);
		return FAILED;
	}
	code = in->get_code(in);
	received_type = in->get_type(in, &received_vendor);
	DBG1(DBG_IKE, "received tunneled EAP-TTLS AVP [EAP/%N/%N]",
							eap_code_short_names, code,
							eap_type_short_names, received_type);
	if (code != EAP_RESPONSE)
	{
		DBG1(DBG_IKE, "%N expected", eap_code_names, EAP_RESPONSE);
		in->destroy(in);
		return FAILED;
	}

	if (this->method)
	{
		type = this->method->get_type(this->method, &vendor);

		if (type != received_type || vendor != received_vendor)
		{
			if (received_vendor == 0 && received_type == EAP_NAK)
			{
				DBG1(DBG_IKE, "peer does not support %N", eap_type_names, type);
			}
			else
			{
				DBG1(DBG_IKE, "received invalid EAP response");
			}
			in->destroy(in);
			return FAILED;
		}
	}

	if (!received_vendor && received_type == EAP_IDENTITY)
	{
		chunk_t eap_id;

		if (this->method == NULL)
		{
			/* Received an EAP Identity response without a matching request */
			this->method = charon->eap->create_instance(charon->eap,
											 EAP_IDENTITY, 0, EAP_SERVER,
											 this->server, this->peer);
			if (this->method == NULL)
			{
				DBG1(DBG_IKE, "%N method not available",
							   eap_type_names, EAP_IDENTITY);
				return FAILED;
			}
		}

		if (this->method->process(this->method, in, &this->out) != SUCCESS)
		{

			DBG1(DBG_IKE, "%N method failed", eap_type_names, EAP_IDENTITY);
			return FAILED;
		}

		if (this->method->get_msk(this->method, &eap_id) == SUCCESS)
		{
			this->peer->destroy(this->peer);
			this->peer = identification_create_from_data(eap_id);
			DBG1(DBG_IKE, "received EAP identity '%Y'", this->peer);
		}

		in->destroy(in);
		this->method->destroy(this->method);
		this->method = NULL;

		/* Start Phase 2 of EAP-TTLS authentication */
		if (lib->settings->get_bool(lib->settings,
					"%s.plugins.eap-ttls.request_peer_auth", FALSE, lib->ns))
		{
			return start_phase2_tnc(this, EAP_TLS);
		}
		else
		{
			return start_phase2_auth(this);
		}
	}

	if (this->method == 0)
	{
		DBG1(DBG_IKE, "no %N phase2 method installed", eap_type_names, EAP_TTLS);
		in->destroy(in);
		return FAILED;
	}

	status = this->method->process(this->method, in, &this->out);
	in->destroy(in);

	switch (status)
	{
		case SUCCESS:
			DBG1(DBG_IKE, "%N phase2 authentication of '%Y' with %N successful",
							eap_type_names, EAP_TTLS, this->peer,
							eap_type_names, type);
			this->method->destroy(this->method);
			this->method = NULL;

			/* continue phase2 with EAP-TNC? */
			return start_phase2_tnc(this, type);
		case NEED_MORE:
			break;
		case FAILED:
		default:
			if (vendor)
			{
				DBG1(DBG_IKE, "vendor specific EAP method %d-%d failed",
							   type, vendor);
			}
			else
			{
				DBG1(DBG_IKE, "%N method failed", eap_type_names, type);
			}
			return FAILED;
	}
	return status;
}

METHOD(tls_application_t, build, status_t,
	private_eap_ttls_server_t *this, bio_writer_t *writer)
{
	chunk_t data;
	eap_code_t code;
	eap_type_t type;
	uint32_t vendor;

	if (this->method == NULL && this->start_phase2 &&
		lib->settings->get_bool(lib->settings,
					"%s.plugins.eap-ttls.phase2_piggyback", FALSE, lib->ns))
	{
		/* generate an EAP Identity request which will be piggybacked right
		 * onto the TLS Finished message thus initiating EAP-TTLS phase2
		 */
		this->method = charon->eap->create_instance(charon->eap, EAP_IDENTITY,
								 0,	EAP_SERVER, this->server, this->peer);
		if (this->method == NULL)
		{
			DBG1(DBG_IKE, "%N method not available",
				 eap_type_names, EAP_IDENTITY);
			return FAILED;
		}
		this->method->initiate(this->method, &this->out);
		this->start_phase2 = FALSE;
	}

	if (this->out)
	{
		code = this->out->get_code(this->out);
		type = this->out->get_type(this->out, &vendor);
		DBG1(DBG_IKE, "sending tunneled EAP-TTLS AVP [EAP/%N/%N]",
						eap_code_short_names, code, eap_type_short_names, type);

		/* get the raw EAP message data */
		data = this->out->get_data(this->out);
		this->avp->build(this->avp, writer, data);

		this->out->destroy(this->out);
		this->out = NULL;
	}
	return INVALID_STATE;
}

METHOD(tls_application_t, destroy, void,
	private_eap_ttls_server_t *this)
{
	this->server->destroy(this->server);
	this->peer->destroy(this->peer);
	DESTROY_IF(this->method);
	DESTROY_IF(this->out);
	this->avp->destroy(this->avp);
	free(this);
}

/**
 * See header
 */
eap_ttls_server_t *eap_ttls_server_create(identification_t *server,
										  identification_t *peer)
{
	private_eap_ttls_server_t *this;

	INIT(this,
		.public = {
			.application = {
				.process = _process,
				.build = _build,
				.destroy = _destroy,
			},
		},
		.server = server->clone(server),
		.peer = peer->clone(peer),
		.start_phase2 = TRUE,
		.start_phase2_tnc = TRUE,
		.avp = eap_ttls_avp_create(),
	);

	return &this->public;
}
