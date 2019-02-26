/*
 * Copyright (C) 2011 Andreas Steffen
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

#include "eap_peap_server.h"
#include "eap_peap_avp.h"

#include <utils/debug.h>
#include <daemon.h>

typedef struct private_eap_peap_server_t private_eap_peap_server_t;

/**
 * Private data of an eap_peap_server_t object.
 */
struct private_eap_peap_server_t {

	/**
	 * Public eap_peap_server_t interface.
	 */
	eap_peap_server_t public;

	/**
	 * Server identity
	 */
	identification_t *server;

	/**
	 * Peer identity
	 */
	identification_t *peer;

	/**
	 * Current EAP-PEAP phase2 state
	 */
	bool start_phase2;

	/**
	 * Current EAP-PEAP phase2 TNC state
	 */
	bool start_phase2_tnc;

	/**
	 * Starts phase 2 with EAP Identity request
	 */
	bool start_phase2_id;

	/**
	 * Final EAP-PEAP phase2 result
	 */
	eap_code_t phase2_result;

	/**
     * Outer phase 1 EAP method
	 */
	eap_method_t *ph1_method;

	/**
     * Current phase 2 EAP method
	 */
	eap_method_t *ph2_method;

	/**
     * Pending outbound EAP message
	 */
	eap_payload_t *out;

	/**
	 * AVP handler
	 */
	eap_peap_avp_t *avp;
};

/**
 * Start EAP client authentication protocol
 */
static status_t start_phase2_auth(private_eap_peap_server_t *this)
{
	char *eap_type_str;
	eap_type_t type;

	eap_type_str = lib->settings->get_str(lib->settings,
							"%s.plugins.eap-peap.phase2_method", "mschapv2",
							lib->ns);
	type = eap_type_from_string(eap_type_str);
	if (type == 0)
	{
		DBG1(DBG_IKE, "unrecognized phase2 method \"%s\"", eap_type_str);
		return FAILED;
	}
	DBG1(DBG_IKE, "phase2 method %N selected", eap_type_names, type);
		this->ph2_method = charon->eap->create_instance(charon->eap, type, 0,
								EAP_SERVER, this->server, this->peer);
	if (this->ph2_method == NULL)
	{
		DBG1(DBG_IKE, "%N method not available", eap_type_names, type);
		return FAILED;
	}

	/* synchronize EAP message identifiers of inner protocol with outer */
	this->ph2_method->set_identifier(this->ph2_method,
						this->ph1_method->get_identifier(this->ph1_method) + 1);

	if (this->ph2_method->initiate(this->ph2_method, &this->out) == NEED_MORE)
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
 * If configured, start EAP-TNC protocol
 */
static status_t start_phase2_tnc(private_eap_peap_server_t *this)
{
	if (this->start_phase2_tnc && lib->settings->get_bool(lib->settings,
						"%s.plugins.eap-peap.phase2_tnc", FALSE, lib->ns))
	{
		DBG1(DBG_IKE, "phase2 method %N selected", eap_type_names, EAP_TNC);
		this->ph2_method = charon->eap->create_instance(charon->eap, EAP_TNC,
									0, EAP_SERVER, this->server, this->peer);
		if (this->ph2_method == NULL)
		{
			DBG1(DBG_IKE, "%N method not available", eap_type_names, EAP_TNC);
			return FAILED;
		}
		this->start_phase2_tnc = FALSE;

		/* synchronize EAP message identifiers of inner protocol with outer */
		this->ph2_method->set_identifier(this->ph2_method,
						this->ph1_method->get_identifier(this->ph1_method) + 1);

		if (this->ph2_method->initiate(this->ph2_method, &this->out) == NEED_MORE)
		{
			return NEED_MORE;
		}
		else
		{
			DBG1(DBG_IKE, "%N method failed", eap_type_names, EAP_TNC);
			return FAILED;
		}
	}
	return SUCCESS;
}

METHOD(tls_application_t, process, status_t,
	private_eap_peap_server_t *this, bio_reader_t *reader)
{
	chunk_t data = chunk_empty;
	status_t status;
	payload_t *payload;
	eap_payload_t *in;
	eap_code_t code;
	eap_type_t type = EAP_NAK, received_type;
	uint32_t vendor, received_vendor;

	status = this->avp->process(this->avp, reader, &data,
							this->ph1_method->get_identifier(this->ph1_method));
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
	DBG3(DBG_IKE, "%B", &data);
	chunk_free(&data);
	payload = (payload_t*)in;

	if (payload->verify(payload) != SUCCESS)
	{
		in->destroy(in);
		return FAILED;
	}

	code = in->get_code(in);
	if (code == EAP_REQUEST || code == EAP_RESPONSE)
	{
		received_type = in->get_type(in, &received_vendor);
		DBG1(DBG_IKE, "received tunneled EAP-PEAP AVP [EAP/%N/%N]",
								eap_code_short_names, code,
								eap_type_short_names, received_type);
		if (code != EAP_RESPONSE)
		{
			DBG1(DBG_IKE, "%N expected", eap_code_names, EAP_RESPONSE);
			in->destroy(in);
			return FAILED;
		}
	}
	else
	{
		DBG1(DBG_IKE, "received tunneled EAP-PEAP AVP [EAP/%N]",
								eap_code_short_names, code);
		in->destroy(in);
		/* if EAP_SUCCESS check if to continue phase2 with EAP-TNC */
		return (this->phase2_result == EAP_SUCCESS && code == EAP_SUCCESS) ?
			   start_phase2_tnc(this) : FAILED;
	}

	if (this->ph2_method)
	{
		type = this->ph2_method->get_type(this->ph2_method, &vendor);

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

		if (this->ph2_method == NULL)
		{
			/* Received an EAP Identity response without a matching request */
			this->ph2_method = charon->eap->create_instance(charon->eap,
											 EAP_IDENTITY, 0, EAP_SERVER,
											 this->server, this->peer);
			if (this->ph2_method == NULL)
			{
				DBG1(DBG_IKE, "%N method not available",
							   eap_type_names, EAP_IDENTITY);
				in->destroy(in);
				return FAILED;
			}
		}

		if (this->ph2_method->process(this->ph2_method, in, &this->out) != SUCCESS)
		{

			DBG1(DBG_IKE, "%N method failed", eap_type_names, EAP_IDENTITY);
			in->destroy(in);
			return FAILED;
		}

		if (this->ph2_method->get_msk(this->ph2_method, &eap_id) == SUCCESS)
		{
			this->peer->destroy(this->peer);
			this->peer = identification_create_from_data(eap_id);
			DBG1(DBG_IKE, "received EAP identity '%Y'", this->peer);
		}

		in->destroy(in);
		this->ph2_method->destroy(this->ph2_method);
		this->ph2_method = NULL;

		/* Start Phase 2 of EAP-PEAP authentication */
		if (lib->settings->get_bool(lib->settings,
					"%s.plugins.eap-peap.request_peer_auth", FALSE, lib->ns))
		{
			return start_phase2_tnc(this);
		}
		else
		{
			return start_phase2_auth(this);
		}
	}

	if (this->ph2_method == 0)
	{
		DBG1(DBG_IKE, "no %N phase2 method installed", eap_type_names, EAP_PEAP);
		in->destroy(in);
		return FAILED;
	}

	status = this->ph2_method->process(this->ph2_method, in, &this->out);
	in->destroy(in);

	switch (status)
	{
		case SUCCESS:
			DBG1(DBG_IKE, "%N phase2 authentication of '%Y' with %N successful",
							eap_type_names, EAP_PEAP, this->peer,
							eap_type_names, type);
			this->ph2_method->destroy(this->ph2_method);
			this->ph2_method = NULL;

			/* EAP-PEAP requires the sending of an inner EAP_SUCCESS message */
			this->phase2_result = EAP_SUCCESS;
			this->out = eap_payload_create_code(this->phase2_result, 1 +
							this->ph1_method->get_identifier(this->ph1_method));
			return NEED_MORE;
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
			/* EAP-PEAP requires the sending of an inner EAP_FAILURE message */
			this->phase2_result = EAP_FAILURE;
			this->out = eap_payload_create_code(this->phase2_result, 1 +
							this->ph1_method->get_identifier(this->ph1_method));
			return NEED_MORE;
	}
	return status;
}

METHOD(tls_application_t, build, status_t,
	private_eap_peap_server_t *this, bio_writer_t *writer)
{
	chunk_t data;
	eap_code_t code;
	eap_type_t type;
	uint32_t vendor;

	if (this->ph2_method == NULL && this->start_phase2 && this->start_phase2_id)
	{
		/*
		 * Start Phase 2 with an EAP Identity request either piggybacked right
		 * onto the TLS Finished payload or delayed after the reception of an
		 * empty EAP Acknowledge message.
		 */
		this->ph2_method = charon->eap->create_instance(charon->eap, EAP_IDENTITY,
								 0,	EAP_SERVER, this->server, this->peer);
		if (this->ph2_method == NULL)
		{
			DBG1(DBG_IKE, "%N method not available",
				 eap_type_names, EAP_IDENTITY);
			return FAILED;
		}

		/* synchronize EAP message identifiers of inner protocol with outer */
		this->ph2_method->set_identifier(this->ph2_method,
					this->ph1_method->get_identifier(this->ph1_method));

		this->ph2_method->initiate(this->ph2_method, &this->out);
		this->start_phase2 = FALSE;
	}

	this->start_phase2_id = TRUE;

	if (this->out)
	{
		code = this->out->get_code(this->out);
		type = this->out->get_type(this->out, &vendor);
		if (code == EAP_REQUEST || code == EAP_RESPONSE)
		{
			DBG1(DBG_IKE, "sending tunneled EAP-PEAP AVP [EAP/%N/%N]",
				 eap_code_short_names, code, eap_type_short_names, type);
		}
		else
		{
			DBG1(DBG_IKE, "sending tunneled EAP-PEAP AVP [EAP/%N]",
				 eap_code_short_names, code);
		}

		/* get the raw EAP message data */
		data = this->out->get_data(this->out);
		DBG3(DBG_IKE, "%B", &data);
		this->avp->build(this->avp, writer, data);

		this->out->destroy(this->out);
		this->out = NULL;
	}
	return INVALID_STATE;
}

METHOD(tls_application_t, destroy, void,
	private_eap_peap_server_t *this)
{
	this->server->destroy(this->server);
	this->peer->destroy(this->peer);
	DESTROY_IF(this->ph2_method);
	DESTROY_IF(this->out);
	this->avp->destroy(this->avp);
	free(this);
}

/**
 * See header
 */
eap_peap_server_t *eap_peap_server_create(identification_t *server,
										  identification_t *peer,
										  eap_method_t *eap_method)
{
	private_eap_peap_server_t *this;

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
		.ph1_method = eap_method,
		.start_phase2 = TRUE,
		.start_phase2_tnc = TRUE,
		.start_phase2_id = lib->settings->get_bool(lib->settings,
										"%s.plugins.eap-peap.phase2_piggyback",
										FALSE, lib->ns),
		.phase2_result = EAP_FAILURE,
		.avp = eap_peap_avp_create(TRUE),
	);

	return &this->public;
}
