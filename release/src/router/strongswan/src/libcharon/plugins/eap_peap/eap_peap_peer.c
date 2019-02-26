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

#include "eap_peap_peer.h"
#include "eap_peap_avp.h"

#include <utils/debug.h>
#include <daemon.h>

typedef struct private_eap_peap_peer_t private_eap_peap_peer_t;

/**
 * Private data of an eap_peap_peer_t object.
 */
struct private_eap_peap_peer_t {

	/**
	 * Public eap_peap_peer_t interface.
	 */
	eap_peap_peer_t public;

	/**
	 * Server identity
	 */
	identification_t *server;

	/**
	 * Peer identity
	 */
	identification_t *peer;

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

METHOD(tls_application_t, process, status_t,
	private_eap_peap_peer_t *this, bio_reader_t *reader)
{
	chunk_t data = chunk_empty;
	status_t status;
	payload_t *payload;
	eap_payload_t *in;
	eap_code_t code;
	eap_type_t type, received_type;
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
		if (code != EAP_REQUEST)
		{
			DBG1(DBG_IKE, "%N expected", eap_code_names, EAP_REQUEST);
			in->destroy(in);
			return FAILED;
		}
	}
	else
	{
		DBG1(DBG_IKE, "received tunneled EAP-PEAP AVP [EAP/%N]",
						   		eap_code_short_names, code);
		this->out = eap_payload_create_code(code, in->get_identifier(in));
		in->destroy(in);
		return NEED_MORE;
	}

	/* yet another phase2 authentication? */
	if (this->ph2_method)
	{
		type = this->ph2_method->get_type(this->ph2_method, &vendor);

		if (type != received_type || vendor != received_vendor)
		{
			this->ph2_method->destroy(this->ph2_method);
			this->ph2_method = NULL;
		}
	}

	if (this->ph2_method == NULL)
	{
		if (received_vendor)
		{
			DBG1(DBG_IKE, "server requested vendor specific EAP method %d-%d "
						  "(id 0x%02X", received_type, received_vendor,
						   in->get_identifier(in));
		}
		else
		{
			DBG1(DBG_IKE, "server requested %N authentication (id 0x%02X)",
				 eap_type_names, received_type, in->get_identifier(in));
		}
		this->ph2_method = charon->eap->create_instance(charon->eap,
									received_type, received_vendor,
									EAP_PEER, this->server, this->peer);
		if (!this->ph2_method)
		{
			DBG1(DBG_IKE, "EAP method not supported");
			this->out = eap_payload_create_nak(in->get_identifier(in), 0, 0,
											   in->is_expanded(in));
			in->destroy(in);
			return NEED_MORE;
		}
		type = this->ph2_method->get_type(this->ph2_method, &vendor);
	}

	status = this->ph2_method->process(this->ph2_method, in, &this->out);
	in->destroy(in);

	switch (status)
	{
		case SUCCESS:
			this->ph2_method->destroy(this->ph2_method);
			this->ph2_method = NULL;
			/* fall through to NEED_MORE */
		case NEED_MORE:
			return NEED_MORE;
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
}

METHOD(tls_application_t, build, status_t,
	private_eap_peap_peer_t *this, bio_writer_t *writer)
{
	chunk_t data;
	eap_code_t code;
	eap_type_t type;
	uint32_t vendor;

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
	private_eap_peap_peer_t *this)
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
eap_peap_peer_t *eap_peap_peer_create(identification_t *server,
									  identification_t *peer,
									  eap_method_t *eap_method)
{
	private_eap_peap_peer_t *this;

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
		.avp = eap_peap_avp_create(FALSE),
	);

	return &this->public;
}
