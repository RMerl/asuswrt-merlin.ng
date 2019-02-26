/*
 * Copyright (C) 2010 Sansar Choinyanbuu
 * Copyright (C) 2010-2015 Andreas Steffen
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

#include "tnccs_20.h"
#include "tnccs_20_handler.h"
#include "tnccs_20_server.h"
#include "tnccs_20_client.h"
#include "batch/pb_tnc_batch.h"
#include "messages/pb_tnc_msg.h"
#include "messages/ietf/pb_pa_msg.h"

#include <tncif_names.h>
#include <tncif_pa_subtypes.h>

#include <utils/debug.h>

typedef struct private_tnccs_20_t private_tnccs_20_t;

/**
 * Private data of a tnccs_20_t object.
 */
struct private_tnccs_20_t {

	/**
	 * Public tnccs_t interface.
	 */
	tnccs_t public;

	/**
	 * TNCC if TRUE, TNCS if FALSE
	 */
	bool is_server;

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
	 * Underlying TNC IF-T transport protocol
	 */
	tnc_ift_type_t transport;

	/**
	 *  TNC IF-T transport protocol for EAP methods
	 */
	bool eap_transport;

	/**
	 * Type of TNC client authentication
	 */
	uint32_t auth_type;

	/**
	 * Mutual PB-TNC protocol enabled
	 */
	bool mutual;

	/**
	 * Direction the next batch will go to
	 */
	bool to_server;

	/**
	 * TNC Server
	 */
	tnccs_20_handler_t *tnc_server;

	/**
	 * TNC Client
	 */
	tnccs_20_handler_t *tnc_client;

	/**
	 * Active TNCSS handler
	 */
	tnccs_20_handler_t *tnccs_handler;

	/**
	 * Maximum PB-TNC batch size
	 */
	size_t max_batch_len;

	/**
	 * Maximum PA-TNC message size
	 */
	size_t max_msg_len;

	/**
	 * Callback function to communicate recommendation (TNC Server only)
	 */
	tnccs_cb_t callback;

	/**
	 * reference count
	 */
	refcount_t ref;

};

METHOD(tls_t, is_complete, bool,
	private_tnccs_20_t *this)
{
	TNC_IMV_Action_Recommendation rec;
	TNC_IMV_Evaluation_Result eval;
	tnccs_20_server_t *tnc_server;

	if (this->tnc_server)
	{
		tnc_server = (tnccs_20_server_t*)this->tnc_server;
		if (tnc_server->have_recommendation(tnc_server, &rec, &eval))
		{
			return this->callback ? this->callback(rec, eval) : TRUE;
		}
	}
	return FALSE;
}

METHOD(tnccs_t, send_msg, TNC_Result,
	private_tnccs_20_t* this, TNC_IMCID imc_id, TNC_IMVID imv_id,
							  TNC_UInt32 msg_flags,
							  TNC_BufferReference msg,
							  TNC_UInt32 msg_len,
							  TNC_VendorID msg_vid,
							  TNC_MessageSubtype msg_subtype)
{
	pb_tnc_msg_t *pb_tnc_msg;
	enum_name_t *pa_subtype_names;
	bool excl;

	if (!this->tnccs_handler->get_send_flag(this->tnccs_handler))
	{
		DBG1(DBG_TNC, "%s %u not allowed to call SendMessage()",
					   this->to_server ? "IMC" : "IMV",
					   this->to_server ? imc_id : imv_id);

		return TNC_RESULT_ILLEGAL_OPERATION;
	}
	excl = (msg_flags & TNC_MESSAGE_FLAGS_EXCLUSIVE) != 0;

	pb_tnc_msg = pb_pa_msg_create(msg_vid, msg_subtype, imc_id, imv_id,
								  excl, chunk_create(msg, msg_len));

	pa_subtype_names = get_pa_subtype_names(msg_vid);
	if (pa_subtype_names)
	{
		DBG2(DBG_TNC, "creating PB-PA message type '%N/%N' 0x%06x/0x%08x",
					   pen_names, msg_vid, pa_subtype_names, msg_subtype,
					   msg_vid, msg_subtype);
	}
	else
	{
		DBG2(DBG_TNC, "creating PB-PA message type '%N' 0x%06x/0x%08x",
					   pen_names, msg_vid, msg_vid, msg_subtype);
	}
	this->tnccs_handler->add_msg(this->tnccs_handler, pb_tnc_msg);

	return TNC_RESULT_SUCCESS;
}

METHOD(tls_t, process, status_t,
	private_tnccs_20_t *this, void *buf, size_t buflen)
{
	pb_tnc_batch_t *batch;
	bool from_server, fatal_header_error = FALSE;
	status_t status;
	chunk_t data;

	/* On arrival of first batch from TNC client create TNC server */
	if (this->is_server && !this->tnc_server)
	{
		this->tnc_server = tnccs_20_server_create(&this->public, _send_msg,
									this->max_batch_len, this->max_msg_len,
									this->eap_transport);
		if (!this->tnc_server)
		{
			return FAILED;
		}
		this->tnccs_handler = this->tnc_server;
		this->tnccs_handler->begin_handshake(this->tnccs_handler, FALSE);
	}

	data = chunk_create(buf, buflen);
	DBG1(DBG_TNC, "received TNCCS batch (%u bytes)", data.len);
	DBG3(DBG_TNC, "%B", &data);

	/* Parse the header of the received PB-TNC batch */
	batch = pb_tnc_batch_create_from_data(data);
	status = batch->process_header(batch, !this->mutual, this->is_server,
								   &from_server);
	if (status == FAILED)
	{
		fatal_header_error = TRUE;
		status = VERIFY_ERROR;
	}
	this->to_server = this->mutual ? from_server : !this->is_server;

	/* In the mutual case, first batch from TNC server requires a TNC client */
	if (this->to_server && !this->tnc_client)
	{
		this->tnc_client = tnccs_20_client_create(&this->public, _send_msg,
									this->max_batch_len, this->max_msg_len);
		if (!this->tnc_client)
		{
			batch->destroy(batch);
			return FAILED;
		}
		this->tnccs_handler = this->tnc_client;
		this->tnccs_handler->begin_handshake(this->tnccs_handler, this->mutual);
	}
	else
	{
		/* Set active TNCCS handler for processing */
		this->tnccs_handler = this->to_server ? this->tnc_client :
												this->tnc_server;
	}
	DBG2(DBG_TNC, "TNC %s is handling inbound connection",
				   this->to_server ? "client" : "server");

	if (status == SUCCESS)
	{
		status = this->tnccs_handler->process(this->tnccs_handler, batch);
	}
	if (status == VERIFY_ERROR)
	{
		this->tnccs_handler->handle_errors(this->tnccs_handler, batch,
										   fatal_header_error);
		status = NEED_MORE;
	}
	batch->destroy(batch);

	/* Has a mutual connection been established? */
	this->mutual = this->is_server ?
				   this->tnc_server->get_mutual(this->tnc_server) :
				   this->tnc_client->get_mutual(this->tnc_client);

	if (this->mutual && !this->is_server)
	{
		pb_tnc_state_t client_state, server_state;

		client_state = !this->tnc_client ? PB_STATE_INIT :
						this->tnc_client->get_state(this->tnc_client);
		server_state = !this->tnc_server ? PB_STATE_INIT :
						this->tnc_server->get_state(this->tnc_server);

		/* In half-duplex mutual mode toggle the direction on the client side */
		if ((!this->to_server && client_state != PB_STATE_DECIDED) ||
			( this->to_server && server_state != PB_STATE_END))
		{
			this->to_server = !this->to_server;
		}
		else if (client_state == PB_STATE_DECIDED &&
				 server_state == PB_STATE_END)
		{
			/* Cause the final CLOSE batch to be sent to the TNC server */
			this->to_server = TRUE;
		}

		/* Suppress a successful CLOSE batch coming from the TNC server */
		if (status == SUCCESS)
		{
			is_complete(this);
			status = NEED_MORE;
		}
	}

	return status;
}

METHOD(tls_t, build, status_t,
	private_tnccs_20_t *this, void *buf, size_t *buflen, size_t *msglen)
{
	if (this->to_server)
	{
		DBG2(DBG_TNC, "TNC client is handling outbound connection");

		/* Before sending the first PB-TNC batch create TNC client */
		if (this->tnc_client)
		{
			this->tnccs_handler = this->tnc_client;
		}
		else
		{
			this->tnc_client = tnccs_20_client_create(&this->public, _send_msg,
													  this->max_batch_len,
													  this->max_msg_len);
			if (!this->tnc_client)
			{
				return FAILED;
			}
			this->tnccs_handler = this->tnc_client;
			this->tnccs_handler->begin_handshake(this->tnccs_handler,
												 this->mutual);
		}
	}
	else
	{
		DBG2(DBG_TNC, "TNC server is handling outbound connection");

		/* Before sending the first PB-TNC batch create TNC server */
		if (this->tnc_server)
		{
			this->tnccs_handler = this->tnc_server;
		}
		else
		{
			this->tnc_server = tnccs_20_server_create(&this->public, _send_msg,
										this->max_batch_len, this->max_msg_len,
										this->eap_transport);
			if (!this->tnc_server)
			{
				return FAILED;
			}
			this->tnccs_handler = this->tnc_server;
			this->tnccs_handler->begin_handshake(this->tnccs_handler,
												 this->mutual);
		}
	}
	return this->tnccs_handler->build(this->tnccs_handler, buf, buflen, msglen);
}

METHOD(tls_t, is_server, bool,
	private_tnccs_20_t *this)
{
	return this->is_server;
}

METHOD(tls_t, get_server_id, identification_t*,
	private_tnccs_20_t *this)
{
	return this->server_id;
}

METHOD(tls_t, set_peer_id, void,
	private_tnccs_20_t *this, identification_t *id)
{
	DESTROY_IF(this->peer_id);
	this->peer_id = id->clone(id);
}

METHOD(tls_t, get_peer_id, identification_t*,
	private_tnccs_20_t *this)
{
	return this->peer_id;
}

METHOD(tls_t, get_purpose, tls_purpose_t,
	private_tnccs_20_t *this)
{
	return TLS_PURPOSE_EAP_TNC;
}

METHOD(tls_t, get_eap_msk, chunk_t,
	private_tnccs_20_t *this)
{
	return chunk_empty;
}

METHOD(tls_t, destroy, void,
	private_tnccs_20_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->tnc_server);
		DESTROY_IF(this->tnc_client);
		this->server_id->destroy(this->server_id);
		this->peer_id->destroy(this->peer_id);
		this->server_ip->destroy(this->server_ip);
		this->peer_ip->destroy(this->peer_ip);
		free(this);
	}
}

METHOD(tnccs_t, get_server_ip, host_t*,
	private_tnccs_20_t *this)
{
	return this->server_ip;
}

METHOD(tnccs_t, get_peer_ip, host_t*,
	private_tnccs_20_t *this)
{
	return this->peer_ip;
}

METHOD(tnccs_t, get_transport, tnc_ift_type_t,
	private_tnccs_20_t *this)
{
	return this->transport;
}

METHOD(tnccs_t, set_transport, void,
	private_tnccs_20_t *this, tnc_ift_type_t transport)
{
	this->transport = transport;
}

METHOD(tnccs_t, get_auth_type, uint32_t,
	private_tnccs_20_t *this)
{
	return this->auth_type;
}

METHOD(tnccs_t, set_auth_type, void,
	private_tnccs_20_t *this, uint32_t auth_type)
{
	this->auth_type = auth_type;
}

METHOD(tnccs_t, get_pdp_server, chunk_t,
	private_tnccs_20_t *this, uint16_t *port)
{
	if (this->tnc_client)
	{
		tnccs_20_client_t *tnc_client;

		tnc_client = (tnccs_20_client_t*)this->tnc_client;

		return tnc_client->get_pdp_server(tnc_client, port);
	}
	else
	{
		*port = 0;
		return chunk_empty;
	}
}

METHOD(tnccs_t, get_ref, tnccs_t*,
	private_tnccs_20_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

/**
 * See header
 */
tnccs_t* tnccs_20_create(bool is_server, identification_t *server_id,
						 identification_t *peer_id, host_t *server_ip,
						 host_t *peer_ip, tnc_ift_type_t transport,
						 tnccs_cb_t cb)
{
	private_tnccs_20_t *this;
	size_t max_batch_size, default_max_batch_size;
	size_t max_message_size, default_max_message_size;

	/* Determine the maximum PB-TNC batch size and PA-TNC message size */
	switch (transport)
	{
		case TNC_IFT_TLS_2_0:
		case TNC_IFT_TLS_1_0:
			default_max_batch_size = 128 * TLS_MAX_FRAGMENT_LEN - 16;
			break;
		case TNC_IFT_EAP_2_0:
		case TNC_IFT_EAP_1_1:
		case TNC_IFT_EAP_1_0:
		case TNC_IFT_UNKNOWN:
		default:
			default_max_batch_size =   4 * TLS_MAX_FRAGMENT_LEN - 14;
			break;
	}

	max_batch_size = min(default_max_batch_size,
						lib->settings->get_int(lib->settings,
								"%s.plugins.tnccs-20.max_batch_size",
								 default_max_batch_size, lib->ns));

	default_max_message_size = max_batch_size - PB_TNC_BATCH_HEADER_SIZE
											  - PB_TNC_MSG_HEADER_SIZE
											  - PB_PA_MSG_HEADER_SIZE;

	max_message_size = min(default_max_message_size,
							lib->settings->get_int(lib->settings,
								"%s.plugins.tnccs-20.max_message_size",
								 default_max_message_size, lib->ns));

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
		.is_server = is_server,
		.to_server = !is_server,
		.server_id = server_id->clone(server_id),
		.peer_id = peer_id->clone(peer_id),
		.server_ip = server_ip->clone(server_ip),
		.peer_ip = peer_ip->clone(peer_ip),
		.transport = transport,
		.eap_transport = transport == TNC_IFT_EAP_1_1 ||
						 transport == TNC_IFT_EAP_2_0,
		.callback = cb,
		.max_batch_len = max_batch_size,
		.max_msg_len = max_message_size,
		.ref = 1,
	);

	return &this->public;
}
