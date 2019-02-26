/*
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

#include "eap_ttls_peer.h"
#include "eap_ttls_avp.h"

#include <utils/debug.h>
#include <daemon.h>
#include <radius_message.h>
#include <sa/eap/eap_method.h>

typedef struct private_eap_ttls_peer_t private_eap_ttls_peer_t;

/**
 * Private data of an eap_ttls_peer_t object.
 */
struct private_eap_ttls_peer_t {

	/**
	 * Public eap_ttls_peer_t interface.
	 */
	eap_ttls_peer_t public;

	/**
	 * Server identity
	 */
	identification_t *server;

	/**
	 * Peer identity
	 */
	identification_t *peer;

	/**
	 * Current EAP-TTLS state
	 */
	bool start_phase2;

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

METHOD(tls_application_t, process, status_t,
	private_eap_ttls_peer_t *this, bio_reader_t *reader)
{
	chunk_t avp_data = chunk_empty;
	chunk_t eap_data = chunk_empty;
	status_t status;
	payload_t *payload;
	eap_payload_t *in;
	eap_packet_t *pkt;
	eap_code_t code;
	eap_type_t type, received_type;
	uint32_t vendor, received_vendor;
	uint16_t eap_len;
	size_t eap_pos = 0;
	bool concatenated = FALSE;

	do
	{
		status = this->avp->process(this->avp, reader, &avp_data);
		switch (status)
		{
			case SUCCESS:
				break;
			case NEED_MORE:
				DBG1(DBG_IKE, "need more AVP data");
				return NEED_MORE;
			case FAILED:
			default:
				return FAILED;
		}

		if (eap_data.len == 0)
		{
			if (avp_data.len < 4)
			{
				DBG1(DBG_IKE, "AVP size to small to contain EAP header");
				chunk_free(&avp_data);
				return FAILED;
			}
			pkt = (eap_packet_t*)avp_data.ptr;
			eap_len = untoh16(&pkt->length);

			if (eap_len <= avp_data.len)
			{
				/* standard:  EAP packet contained in a single AVP */
				eap_data = avp_data;
				break;
			}
			else if (eap_len > reader->remaining(reader) + avp_data.len)
			{
				/* rough size check, ignoring AVP headers in remaining data */
				DBG1(DBG_IKE, "EAP packet too large for EAP-TTLS AVP(s)");
				chunk_free(&avp_data);
				return FAILED;
			}
			else if (avp_data.len == MAX_RADIUS_ATTRIBUTE_SIZE)
			{
				/* non-standard: EAP packet segmented into multiple AVPs */
				eap_data = chunk_alloc(eap_len);
				concatenated = TRUE;
			}
			else
			{
				DBG1(DBG_IKE, "non-radius segmentation of EAP packet into AVPs");
				chunk_free(&avp_data);
				return FAILED;
			}
		}

		if (avp_data.len > eap_data.len - eap_pos)
		{
			DBG1(DBG_IKE, "AVP size too large to fit into EAP packet");
			chunk_free(&avp_data);
			chunk_free(&eap_data);
			return FAILED;
		}
		memcpy(eap_data.ptr + eap_pos, avp_data.ptr, avp_data.len);
		eap_pos += avp_data.len;
		chunk_free(&avp_data);
	}
	while (eap_pos < eap_data.len);

	in = eap_payload_create_data(eap_data);
	chunk_free(&eap_data);
	payload = (payload_t*)in;

	if (payload->verify(payload) != SUCCESS)
	{
		in->destroy(in);
		return FAILED;
	}
	code = in->get_code(in);
	received_type = in->get_type(in, &received_vendor);
	DBG1(DBG_IKE, "received tunneled EAP-TTLS AVP%s [EAP/%N/%N]",
							concatenated ? "s" : "",
					   		eap_code_short_names, code,
							eap_type_short_names, received_type);
	if (code != EAP_REQUEST)
	{
		DBG1(DBG_IKE, "%N expected", eap_code_names, EAP_REQUEST);
		in->destroy(in);
		return FAILED;
	}

	/* yet another phase2 authentication? */
	if (this->method)
	{
		type = this->method->get_type(this->method, &vendor);

		if (type != received_type || vendor != received_vendor)
		{
			this->method->destroy(this->method);
			this->method = NULL;
		}
	}

	if (this->method == NULL)
	{
		if (received_vendor)
		{
			DBG1(DBG_IKE, "server requested vendor specific EAP method %d-%d "
						  "(id 0x%02X)", received_type, received_vendor,
						   in->get_identifier(in));
		}
		else
		{
			DBG1(DBG_IKE, "server requested %N authentication (id 0x%02X)",
				 eap_type_names, received_type, in->get_identifier(in));
		}
		this->method = charon->eap->create_instance(charon->eap,
									received_type, received_vendor,
									EAP_PEER, this->server, this->peer);
		if (!this->method)
		{
			DBG1(DBG_IKE, "EAP method not supported");
			this->out = eap_payload_create_nak(in->get_identifier(in), 0, 0,
											   in->is_expanded(in));
			in->destroy(in);
			return NEED_MORE;
		}
		type = this->method->get_type(this->method, &vendor);
		this->start_phase2 = FALSE;
	}

	status = this->method->process(this->method, in, &this->out);
	in->destroy(in);

	switch (status)
	{
		case SUCCESS:
			this->method->destroy(this->method);
			this->method = NULL;
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
	private_eap_ttls_peer_t *this, bio_writer_t *writer)
{
	chunk_t data;
	eap_code_t code;
	eap_type_t type;
	uint32_t vendor;

	if (this->method == NULL && this->start_phase2)
	{
		/* generate an EAP Identity response */
		this->method = charon->eap->create_instance(charon->eap, EAP_IDENTITY,
								 0,	EAP_PEER, this->server, this->peer);
		if (this->method == NULL)
		{
			DBG1(DBG_IKE, "EAP_IDENTITY method not available");
			return FAILED;
		}
		this->method->process(this->method, NULL, &this->out);
		this->method->destroy(this->method);
		this->method = NULL;
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
	private_eap_ttls_peer_t *this)
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
eap_ttls_peer_t *eap_ttls_peer_create(identification_t *server,
									  identification_t *peer)
{
	private_eap_ttls_peer_t *this;

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
		.avp = eap_ttls_avp_create(),
	);

	return &this->public;
}
