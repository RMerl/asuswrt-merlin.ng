/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "hook.h"

#include <sa/ikev2/keymat_v2.h>
#include <encoding/payloads/nonce_payload.h>
#include <encoding/payloads/cert_payload.h>
#include <encoding/payloads/auth_payload.h>
#include <encoding/payloads/id_payload.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/ts_payload.h>

typedef struct private_pretend_auth_t private_pretend_auth_t;

/**
 * Private data of an pretend_auth_t object.
 */
struct private_pretend_auth_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;

	/**
	 * remote peer identity
	 */
	identification_t *id;

	/**
	 * reserved bytes of ID payload
	 */
	char reserved[3];

	/**
	 * IKE_SA_INIT data for signature
	 */
	chunk_t ike_init;

	/**
	 * Nonce for signature
	 */
	chunk_t nonce;

	/**
	 * Selected CHILD_SA proposal
	 */
	proposal_t *proposal;

	/**
	 * List of initiators Traffic Selectors
	 */
	linked_list_t *tsi;

	/**
	 * List of responders Traffic Selectors
	 */
	linked_list_t *tsr;
};

/**
 * Process IKE_SA_INIT request message, outgoing
 */
static void process_init_request(private_pretend_auth_t *this,
								 ike_sa_t *ike_sa, message_t *message)
{
	nonce_payload_t *nonce;

	nonce = (nonce_payload_t*)message->get_payload(message, PLV2_NONCE);
	if (nonce)
	{
		free(this->nonce.ptr);
		this->nonce = nonce->get_nonce(nonce);
	}
}

/**
 * Process IKE_AUTH request message, outgoing
 */
static void process_auth_request(private_pretend_auth_t *this,
								 ike_sa_t *ike_sa, message_t *message)
{
	id_payload_t *id;
	sa_payload_t *sa;
	ts_payload_t *tsi, *tsr;
	linked_list_t *proposals;

	id = (id_payload_t*)message->get_payload(message, PLV2_ID_RESPONDER);
	if (id)
	{
		this->id->destroy(this->id);
		this->id = id->get_identification(id);
	}
	sa = (sa_payload_t*)message->get_payload(message, PLV2_SECURITY_ASSOCIATION);
	if (sa)
	{
		proposals = sa->get_proposals(sa);
		proposals->remove_first(proposals, (void**)&this->proposal);
		if (this->proposal)
		{
			this->proposal->set_spi(this->proposal, htonl(0x12345678));
		}
		proposals->destroy_offset(proposals, offsetof(proposal_t, destroy));
	}
	tsi = (ts_payload_t*)message->get_payload(message,
											  PLV2_TS_INITIATOR);
	if (tsi)
	{
		this->tsi = tsi->get_traffic_selectors(tsi);
	}
	tsr = (ts_payload_t*)message->get_payload(message,
											  PLV2_TS_RESPONDER);
	if (tsr)
	{
		this->tsr = tsr->get_traffic_selectors(tsr);
	}

}

/**
 * Process IKE_SA_INIT response message, incoming
 */
static void process_init_response(private_pretend_auth_t *this,
								  ike_sa_t *ike_sa, message_t *message)
{
	this->ike_init = chunk_clone(message->get_packet_data(message));
}

/**
 * Build CERT payloads
 */
static void build_certs(private_pretend_auth_t *this,
						ike_sa_t *ike_sa, message_t *message, auth_cfg_t *auth)
{
	enumerator_t *enumerator;
	cert_payload_t *payload;
	certificate_t *cert;
	auth_rule_t type;

	/* get subject cert first, then issuing certificates */
	cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
	if (cert)
	{
		payload = cert_payload_create_from_cert(PLV2_CERTIFICATE, cert);
		if (payload)
		{
			DBG1(DBG_IKE, "pretending end entity cert \"%Y\"",
				 cert->get_subject(cert));
			message->add_payload(message, (payload_t*)payload);
		}
	}
	enumerator = auth->create_enumerator(auth);
	while (enumerator->enumerate(enumerator, &type, &cert))
	{
		if (type == AUTH_RULE_IM_CERT)
		{
			payload = cert_payload_create_from_cert(PLV2_CERTIFICATE, cert);
			if (payload)
			{
				DBG1(DBG_IKE, "pretending issuer cert \"%Y\"",
					 cert->get_subject(cert));
				message->add_payload(message, (payload_t*)payload);
			}
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Build faked AUTH payload
 */
static bool build_auth(private_pretend_auth_t *this,
					   ike_sa_t *ike_sa, message_t *message)
{
	chunk_t octets, auth_data;
	private_key_t *private;
	auth_cfg_t *auth;
	auth_payload_t *auth_payload;
	auth_method_t auth_method;
	signature_scheme_t scheme;
	keymat_v2_t *keymat;

	auth = auth_cfg_create();
	private = lib->credmgr->get_private(lib->credmgr, KEY_ANY, this->id, auth);
	build_certs(this, ike_sa, message, auth);
	auth->destroy(auth);
	if (private == NULL)
	{
		DBG1(DBG_CFG, "no private key found for '%Y' to pretend AUTH", this->id);
		return FALSE;
	}

	switch (private->get_type(private))
	{
		case KEY_RSA:
			scheme = SIGN_RSA_EMSA_PKCS1_SHA1;
			auth_method = AUTH_RSA;
			break;
		case KEY_ECDSA:
			/* we try to deduct the signature scheme from the keysize */
			switch (private->get_keysize(private))
			{
				case 256:
					scheme = SIGN_ECDSA_256;
					auth_method = AUTH_ECDSA_256;
					break;
				case 384:
					scheme = SIGN_ECDSA_384;
					auth_method = AUTH_ECDSA_384;
					break;
				case 521:
					scheme = SIGN_ECDSA_521;
					auth_method = AUTH_ECDSA_521;
					break;
				default:
					DBG1(DBG_CFG, "%d bit ECDSA private key size not supported",
							private->get_keysize(private));
					return FALSE;
			}
			break;
		default:
			DBG1(DBG_CFG, "private key of type %N not supported",
					key_type_names, private->get_type(private));
			return FALSE;
	}
	keymat = (keymat_v2_t*)ike_sa->get_keymat(ike_sa);
	if (!keymat->get_auth_octets(keymat, TRUE, this->ike_init,
								 this->nonce, this->id, this->reserved, &octets))
	{
		private->destroy(private);
		return FALSE;
	}
	if (!private->sign(private, scheme, octets, &auth_data))
	{
		chunk_free(&octets);
		private->destroy(private);
		return FALSE;
	}
	auth_payload = auth_payload_create();
	auth_payload->set_auth_method(auth_payload, auth_method);
	auth_payload->set_data(auth_payload, auth_data);
	chunk_free(&auth_data);
	chunk_free(&octets);
	private->destroy(private);
	message->add_payload(message, (payload_t*)auth_payload);
	DBG1(DBG_CFG, "pretending AUTH payload for '%Y' with %N",
		 this->id, auth_method_names, auth_method);
	return TRUE;
}

/**
 * Process IKE_AUTH response message, incoming
 */
static void process_auth_response(private_pretend_auth_t *this,
								  ike_sa_t *ike_sa, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;

	/* check for, and remove AUTHENTICATION_FAILED notify */
	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		notify_payload_t *notify = (notify_payload_t*)payload;

		if (payload->get_type(payload) != PLV2_NOTIFY ||
			notify->get_notify_type(notify) != AUTHENTICATION_FAILED)
		{
			DBG1(DBG_CFG, "no %N notify found, disabling AUTH pretending",
				 notify_type_names, AUTHENTICATION_FAILED);
			enumerator->destroy(enumerator);
			return;
		}
		message->remove_payload_at(message, enumerator);
		payload->destroy(payload);
	}
	enumerator->destroy(enumerator);

	if (!build_auth(this, ike_sa, message))
	{
		message->add_notify(message, TRUE, AUTHENTICATION_FAILED, chunk_empty);
		return;
	}
	message->add_payload(message, (payload_t*)
				id_payload_create_from_identification(PLV2_ID_RESPONDER, this->id));
	if (this->proposal)
	{
		message->add_payload(message, (payload_t*)
					sa_payload_create_from_proposal_v2(this->proposal));
	}
	if (this->tsi)
	{
		message->add_payload(message, (payload_t*)
					ts_payload_create_from_traffic_selectors(TRUE, this->tsi));
	}
	if (this->tsr)
	{
		message->add_payload(message, (payload_t*)
					ts_payload_create_from_traffic_selectors(FALSE, this->tsr));
	}
}

METHOD(listener_t, message, bool,
	private_pretend_auth_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (plain)
	{
		if (incoming)
		{
			if (!message->get_request(message))
			{
				if (message->get_exchange_type(message) == IKE_SA_INIT)
				{
					process_init_response(this, ike_sa, message);
				}
				if (message->get_exchange_type(message) == IKE_AUTH &&
					message->get_message_id(message) == 1)
				{
					process_auth_response(this, ike_sa, message);
				}
			}
		}
		else
		{
			if (message->get_request(message))
			{
				if (message->get_exchange_type(message) == IKE_SA_INIT)
				{
					process_init_request(this, ike_sa, message);
				}
				if (message->get_exchange_type(message) == IKE_AUTH &&
					message->get_message_id(message) == 1)
				{
					process_auth_request(this, ike_sa, message);
				}
			}
		}
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_pretend_auth_t *this)
{
	if (this->tsi)
	{
		this->tsi->destroy_offset(this->tsi, offsetof(traffic_selector_t, destroy));
	}
	if (this->tsr)
	{
		this->tsr->destroy_offset(this->tsr, offsetof(traffic_selector_t, destroy));
	}
	DESTROY_IF(this->proposal);
	this->id->destroy(this->id);
	free(this->ike_init.ptr);
	free(this->nonce.ptr);
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *pretend_auth_hook_create(char *name)
{
	private_pretend_auth_t *this;

	INIT(this,
		.hook = {
			.listener = {
				.message = _message,
			},
			.destroy = _destroy,
		},
		.id = identification_create_from_string(
				conftest->test->get_str(conftest->test,
										"hooks.%s.peer", "%any", name)),
	);

	return &this->hook;
}
