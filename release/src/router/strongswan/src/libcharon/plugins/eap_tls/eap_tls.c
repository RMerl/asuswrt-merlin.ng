/*
 * Copyright (C) 2023 Tobias Brunner
 * Copyright (C) 2010 Martin Willi
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

#include "eap_tls.h"

#include <tls_eap.h>

#include <daemon.h>
#include <library.h>

typedef struct private_eap_tls_t private_eap_tls_t;

/**
 * Private data of an eap_tls_t object.
 */
struct private_eap_tls_t {

	/**
	 * Public interface.
	 */
	eap_tls_t public;

	/**
	 * TLS stack, wrapped by EAP helper below
	 */
	tls_t *tls;

	/**
	 * EAP helper
	 */
	tls_eap_t *tls_eap;

	/**
	 * Whether the "protected success indication" has been sent/received with
	 * TLS 1.3
	 */
	bool indication_sent_received;
};

/** Maximum number of EAP-TLS messages/fragments allowed */
#define MAX_MESSAGE_COUNT 32
/** Default size of a EAP-TLS fragment */
#define MAX_FRAGMENT_LEN 1024

METHOD(eap_method_t, initiate, status_t,
	private_eap_tls_t *this, eap_payload_t **out)
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
	private_eap_tls_t *this, eap_payload_t *in, eap_payload_t **out)
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
	private_eap_tls_t *this, pen_t *vendor)
{
	*vendor = 0;
	return EAP_TLS;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_tls_t *this, chunk_t *msk)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3 ||
		this->indication_sent_received)
	{
		*msk = this->tls_eap->get_msk(this->tls_eap);
		if (msk->len)
		{
			return SUCCESS;
		}
	}
	else
	{
		DBG1(DBG_TLS, "missing protected success indication for EAP-TLS with "
			 "%N", tls_version_names, this->tls->get_version_max(this->tls));
	}
	return FAILED;
}

METHOD(eap_method_t, get_identifier, uint8_t,
	private_eap_tls_t *this)
{
	return this->tls_eap->get_identifier(this->tls_eap);
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_tls_t *this, uint8_t identifier)
{
	this->tls_eap->set_identifier(this->tls_eap, identifier);
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_tls_t *this)
{
	return TRUE;
}

METHOD(eap_method_t, get_auth, auth_cfg_t*,
	private_eap_tls_t *this)
{
	return this->tls_eap->get_auth(this->tls_eap);
}

METHOD(eap_method_t, destroy, void,
	private_eap_tls_t *this)
{
	this->tls_eap->destroy(this->tls_eap);
	free(this);
}

/**
 * Application to send/process the "protected success indication" with TLS 1.3
 * as specified in RFC 9190
 */
typedef struct {

	/**
	 * Public interface
	 */
	tls_application_t public;

	/**
	 * Reference to the EAP-TLS object
	 */
	private_eap_tls_t *this;

	/**
	 * Whether the server sent the indication
	 */
	bool indication_sent;

} eap_tls_app_t;

METHOD(tls_application_t, server_process, status_t,
	eap_tls_app_t *app, bio_reader_t *reader)
{
	/* we don't expect any data from the client, the empty response to our
	 * indication is handled as ACK in tls_eap_t */
	DBG1(DBG_TLS, "peer sent unexpected TLS data");
	return FAILED;
}

METHOD(tls_application_t, server_build, status_t,
	eap_tls_app_t *app, bio_writer_t *writer)
{
	if (app->this->indication_sent_received)
	{
		return SUCCESS;
	}
	if (app->this->tls->get_version_max(app->this->tls) >= TLS_1_3)
	{
		/* build() is called twice when sending the indication, return the same
		 * status but data only once */
		if (app->indication_sent)
		{
			app->this->indication_sent_received = TRUE;
		}
		else
		{	/* send a single 0x00 */
			DBG2(DBG_TLS, "sending protected success indication via TLS");
			writer->write_uint8(writer, 0);
			app->indication_sent = TRUE;
		}
	}
	else
	{
		/* with earlier TLS versions, return INVALID_STATE without data to send
		 * the final handshake messages (returning SUCCESS immediately would
		 * prevent that) */
		app->this->indication_sent_received = TRUE;
	}
	return INVALID_STATE;
}

METHOD(tls_application_t, client_process, status_t,
	eap_tls_app_t *app, bio_reader_t *reader)
{
	uint8_t indication;

	if (app->this->tls->get_version_max(app->this->tls) < TLS_1_3 ||
		app->this->indication_sent_received)
	{
		DBG1(DBG_TLS, "peer sent unexpected TLS data");
		return FAILED;
	}
	if (!reader->read_uint8(reader, &indication) || indication != 0)
	{
		DBG1(DBG_TLS, "received incorrect protected success indication via TLS");
		return FAILED;
	}
	DBG2(DBG_TLS, "received protected success indication via TLS");
	app->this->indication_sent_received = TRUE;
	return NEED_MORE;
}

METHOD(tls_application_t, client_build, status_t,
	eap_tls_app_t *app, bio_writer_t *writer)
{
	if (app->this->tls->get_version_max(app->this->tls) < TLS_1_3 ||
		app->this->indication_sent_received)
	{	/* trigger an empty response/ACK */
		return INVALID_STATE;
	}
	return FAILED;
}

METHOD(tls_application_t, app_destroy, void,
	eap_tls_app_t *this)
{
	free(this);
}

/**
 * Create the server/peer implementation to handle the "protected success
 * indication" with TLS 1.3
 */
tls_application_t *eap_tls_app_create(private_eap_tls_t *this, bool is_server)
{
	eap_tls_app_t *app;

	INIT(app,
		.public = {
			.process = _client_process,
			.build = _client_build,
			.destroy = _app_destroy,
		},
		.this = this,
	);
	if (is_server)
	{
		app->public.process = _server_process;
		app->public.build = _server_build;
	}
	return &app->public;
}

/**
 * Generic private constructor
 */
static eap_tls_t *eap_tls_create(identification_t *server,
								 identification_t *peer, bool is_server)
{
	private_eap_tls_t *this;
	tls_application_t *app;
	size_t frag_size;
	int max_msg_count;
	bool include_length;

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
				.get_auth = _get_auth,
				.destroy = _destroy,
			},
		},
	);

	frag_size = lib->settings->get_int(lib->settings,
					"%s.plugins.eap-tls.fragment_size", MAX_FRAGMENT_LEN,
					lib->ns);
	max_msg_count = lib->settings->get_int(lib->settings,
					"%s.plugins.eap-tls.max_message_count", MAX_MESSAGE_COUNT,
					lib->ns);
	include_length = lib->settings->get_bool(lib->settings,
					"%s.plugins.eap-tls.include_length", TRUE, lib->ns);
	app = eap_tls_app_create(this, is_server);
	this->tls = tls_create(is_server, server, peer, TLS_PURPOSE_EAP_TLS, app,
						   NULL, 0);
	this->tls_eap = tls_eap_create(EAP_TLS, this->tls, frag_size, max_msg_count,
								   include_length);
	if (!this->tls_eap)
	{
		free(this);
		return NULL;
	}
	return &this->public;
}

eap_tls_t *eap_tls_create_server(identification_t *server,
								 identification_t *peer)
{
	return eap_tls_create(server, peer, TRUE);
}

eap_tls_t *eap_tls_create_peer(identification_t *server,
							   identification_t *peer)
{
	return eap_tls_create(server, peer, FALSE);
}
