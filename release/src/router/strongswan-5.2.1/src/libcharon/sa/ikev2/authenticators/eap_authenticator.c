/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2006-2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include "eap_authenticator.h"

#include <daemon.h>
#include <sa/ikev2/keymat_v2.h>
#include <sa/eap/eap_method.h>
#include <encoding/payloads/auth_payload.h>
#include <encoding/payloads/eap_payload.h>

typedef struct private_eap_authenticator_t private_eap_authenticator_t;

/**
 * Private data of an eap_authenticator_t object.
 */
struct private_eap_authenticator_t {

	/**
	 * Public authenticator_t interface.
	 */
	eap_authenticator_t public;

	/**
	 * Assigned IKE_SA
	 */
	ike_sa_t *ike_sa;

	/**
	 * others nonce to include in AUTH calculation
	 */
	chunk_t received_nonce;

	/**
	 * our nonce to include in AUTH calculation
	 */
	chunk_t sent_nonce;

	/**
	 * others IKE_SA_INIT message data to include in AUTH calculation
	 */
	chunk_t received_init;

	/**
	 * our IKE_SA_INIT message data to include in AUTH calculation
	 */
	chunk_t sent_init;

	/**
	 * Reserved bytes of ID payload
	 */
	char reserved[3];

	/**
	 * Current EAP method processing
	 */
	eap_method_t *method;

	/**
	 * MSK used to build and verify auth payload
	 */
	chunk_t msk;

	/**
	 * EAP authentication method completed successfully
	 */
	bool eap_complete;

	/**
	 * Set if we require mutual EAP due EAP-only authentication
	 */
	bool require_mutual;

	/**
	 * authentication payload verified successfully
	 */
	bool auth_complete;

	/**
	 * generated EAP payload
	 */
	eap_payload_t *eap_payload;

	/**
	 * EAP identity of peer
	 */
	identification_t *eap_identity;
};

/**
 * load an EAP method
 */
static eap_method_t *load_method(private_eap_authenticator_t *this,
							eap_type_t type, u_int32_t vendor, eap_role_t role)
{
	identification_t *server, *peer, *aaa;
	auth_cfg_t *auth;

	if (role == EAP_SERVER)
	{
		server = this->ike_sa->get_my_id(this->ike_sa);
		peer = this->ike_sa->get_other_id(this->ike_sa);
		auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);
	}
	else
	{
		server = this->ike_sa->get_other_id(this->ike_sa);
		peer = this->ike_sa->get_my_id(this->ike_sa);
		auth = this->ike_sa->get_auth_cfg(this->ike_sa, TRUE);
	}
	if (this->eap_identity)
	{
		peer = this->eap_identity;
	}
	aaa = auth->get(auth, AUTH_RULE_AAA_IDENTITY);
	if (aaa)
	{
		server = aaa;
	}
	return charon->eap->create_instance(charon->eap, type, vendor,
										role, server, peer);
}

/**
 * Initiate EAP conversation as server
 */
static eap_payload_t* server_initiate_eap(private_eap_authenticator_t *this,
										  bool do_identity)
{
	auth_cfg_t *auth;
	eap_type_t type;
	identification_t *id;
	u_int32_t vendor;
	eap_payload_t *out;
	char *action;

	auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);

	/* initiate EAP-Identity exchange if required */
	if (!this->eap_identity && do_identity)
	{
		id = auth->get(auth, AUTH_RULE_EAP_IDENTITY);
		if (id)
		{
			if (id->get_type(id) == ID_ANY)
			{
				this->method = load_method(this, EAP_IDENTITY, 0, EAP_SERVER);
				if (this->method)
				{
					if (this->method->initiate(this->method, &out) == NEED_MORE)
					{
						DBG1(DBG_IKE, "initiating %N method (id 0x%02X)",
							 eap_type_names, EAP_IDENTITY,
							 this->method->get_identifier(this->method));
						return out;
					}
					this->method->destroy(this->method);
				}
				DBG1(DBG_IKE, "EAP-Identity request configured, "
					 "but not supported");
			}
			else
			{
				DBG1(DBG_IKE, "using configured EAP-Identity %Y", id);
				this->eap_identity = id->clone(id);
			}
		}
	}
	/* invoke real EAP method */
	type = (uintptr_t)auth->get(auth, AUTH_RULE_EAP_TYPE);
	vendor = (uintptr_t)auth->get(auth, AUTH_RULE_EAP_VENDOR);
	action = "loading";
	this->method = load_method(this, type, vendor, EAP_SERVER);
	if (this->method)
	{
		action = "initiating";
		if (this->method->initiate(this->method, &out) == NEED_MORE)
		{
			type = this->method->get_type(this->method, &vendor);
			if (vendor)
			{
				DBG1(DBG_IKE, "initiating EAP vendor type %d-%d method (id 0x%02X)",
					 type, vendor, out->get_identifier(out));
			}
			else
			{
				DBG1(DBG_IKE, "initiating %N method (id 0x%02X)", eap_type_names,
					 type, out->get_identifier(out));
			}
			return out;
		}
		/* type might have changed for virtual methods */
		type = this->method->get_type(this->method, &vendor);
	}
	if (vendor)
	{
		DBG1(DBG_IKE, "%s EAP vendor type %d-%d method failed",
					  action, type,	vendor);
	}
	else
	{
		DBG1(DBG_IKE, "%s %N method failed", action, eap_type_names, type);
	}
	return eap_payload_create_code(EAP_FAILURE, 0);
}

/**
 * Replace the existing EAP-Identity in other auth config
 */
static void replace_eap_identity(private_eap_authenticator_t *this)
{
	identification_t *eap_identity;
	auth_cfg_t *cfg;

	eap_identity = this->eap_identity->clone(this->eap_identity);
	cfg = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);
	cfg->add(cfg, AUTH_RULE_EAP_IDENTITY, eap_identity);
}

/**
 * Handle EAP exchange as server
 */
static eap_payload_t* server_process_eap(private_eap_authenticator_t *this,
										 eap_payload_t *in)
{
	eap_type_t type, received_type, conf_type;
	u_int32_t vendor, received_vendor, conf_vendor;
	eap_payload_t *out;
	auth_cfg_t *auth;

	if (in->get_code(in) != EAP_RESPONSE)
	{
		DBG1(DBG_IKE, "received %N, sending %N",
			 eap_code_names, in->get_code(in), eap_code_names, EAP_FAILURE);
		return eap_payload_create_code(EAP_FAILURE, in->get_identifier(in));
	}

	type = this->method->get_type(this->method, &vendor);
	received_type = in->get_type(in, &received_vendor);
	if (type != received_type || vendor != received_vendor)
	{
		if (received_vendor == 0 && received_type == EAP_NAK)
		{
			auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);
			conf_type = (uintptr_t)auth->get(auth, AUTH_RULE_EAP_TYPE);
			conf_vendor = (uintptr_t)auth->get(auth, AUTH_RULE_EAP_VENDOR);
			if ((type == EAP_IDENTITY && !vendor) ||
				(type == conf_type && vendor == conf_vendor))
			{
				DBG1(DBG_IKE, "received %N, sending %N",
					 eap_type_names, EAP_NAK, eap_code_names, EAP_FAILURE);
				return eap_payload_create_code(EAP_FAILURE,
											   in->get_identifier(in));
			}
			/* virtual methods handle NAKs in process() */
		}
		else
		{
			DBG1(DBG_IKE, "received invalid EAP response, sending %N",
				 eap_code_names, EAP_FAILURE);
			return eap_payload_create_code(EAP_FAILURE, in->get_identifier(in));
		}
	}

	switch (this->method->process(this->method, in, &out))
	{
		case NEED_MORE:
			return out;
		case SUCCESS:
			if (!vendor && type == EAP_IDENTITY)
			{
				chunk_t data;

				if (this->method->get_msk(this->method, &data) == SUCCESS)
				{
					this->eap_identity = identification_create_from_data(data);
					DBG1(DBG_IKE, "received EAP identity '%Y'",
						 this->eap_identity);
					replace_eap_identity(this);
				}
				/* restart EAP exchange, but with real method */
				this->method->destroy(this->method);
				return server_initiate_eap(this, FALSE);
			}
			if (this->method->get_msk(this->method, &this->msk) == SUCCESS)
			{
				this->msk = chunk_clone(this->msk);
			}
			if (vendor)
			{
				DBG1(DBG_IKE, "EAP vendor specific method %d-%d succeeded, "
					 "%sMSK established", type, vendor,
					 this->msk.ptr ? "" : "no ");
			}
			else
			{
				DBG1(DBG_IKE, "EAP method %N succeeded, %sMSK established",
					 eap_type_names, type, this->msk.ptr ? "" : "no ");
			}
			this->ike_sa->set_condition(this->ike_sa, COND_EAP_AUTHENTICATED,
										TRUE);
			this->eap_complete = TRUE;
			return eap_payload_create_code(EAP_SUCCESS, in->get_identifier(in));
		case FAILED:
		default:
			/* type might have changed for virtual methods */
			type = this->method->get_type(this->method, &vendor);
			if (vendor)
			{
				DBG1(DBG_IKE, "EAP vendor specific method %d-%d failed for "
					 "peer %Y", type, vendor,
					 this->ike_sa->get_other_id(this->ike_sa));
			}
			else
			{
				DBG1(DBG_IKE, "EAP method %N failed for peer %Y",
					 eap_type_names, type,
					 this->ike_sa->get_other_id(this->ike_sa));
			}
			return eap_payload_create_code(EAP_FAILURE, in->get_identifier(in));
	}
}

/**
 * Processing method for a peer
 */
static eap_payload_t* client_process_eap(private_eap_authenticator_t *this,
										 eap_payload_t *in)
{
	eap_type_t type, conf_type;
	u_int32_t vendor, conf_vendor;
	auth_cfg_t *auth;
	eap_payload_t *out;
	identification_t *id;

	type = in->get_type(in, &vendor);

	if (!vendor && type == EAP_IDENTITY)
	{
		DESTROY_IF(this->eap_identity);
		auth = this->ike_sa->get_auth_cfg(this->ike_sa, TRUE);
		id = auth->get(auth, AUTH_RULE_EAP_IDENTITY);
		if (!id || id->get_type(id) == ID_ANY)
		{
			id = this->ike_sa->get_my_id(this->ike_sa);
		}
		DBG1(DBG_IKE, "server requested %N (id 0x%02X), sending '%Y'",
			 eap_type_names, type, in->get_identifier(in), id);
		this->eap_identity = id->clone(id);

		this->method = load_method(this, type, vendor, EAP_PEER);
		if (this->method)
		{
			if (this->method->process(this->method, in, &out) == SUCCESS)
			{
				this->method->destroy(this->method);
				this->method = NULL;
				return out;
			}
			this->method->destroy(this->method);
			this->method = NULL;
		}
		/* FIXME: sending a Nak is not correct here as EAP_IDENTITY (1) is no
		 * EAP method (types 3-253, 255) */
		DBG1(DBG_IKE, "%N not supported, sending EAP_NAK",
			 eap_type_names, type);
		return eap_payload_create_nak(in->get_identifier(in), 0, 0, FALSE);
	}
	if (this->method == NULL)
	{
		if (vendor)
		{
			DBG1(DBG_IKE, "server requested vendor specific EAP method %d-%d ",
						  "(id 0x%02X)", type, vendor, in->get_identifier(in));
		}
		else
		{
			DBG1(DBG_IKE, "server requested %N authentication (id 0x%02X)",
				 eap_type_names, type, in->get_identifier(in));
		}
		auth = this->ike_sa->get_auth_cfg(this->ike_sa, TRUE);
		conf_type = (uintptr_t)auth->get(auth, AUTH_RULE_EAP_TYPE);
		conf_vendor = (uintptr_t)auth->get(auth, AUTH_RULE_EAP_VENDOR);
		if (conf_type != EAP_NAK &&
		   (conf_type != type || conf_vendor != vendor))
		{
			if (conf_vendor)
			{
				DBG1(DBG_IKE, "requesting EAP method %d-%d, sending EAP_NAK",
					 conf_type, conf_vendor);
			}
			else
			{
				DBG1(DBG_IKE, "requesting %N authentication, sending EAP_NAK",
					 eap_type_names, conf_type);
			}
			return eap_payload_create_nak(in->get_identifier(in), conf_type,
										  conf_vendor, in->is_expanded(in));
		}
		this->method = load_method(this, type, vendor, EAP_PEER);
		if (!this->method)
		{
			DBG1(DBG_IKE, "EAP method not supported, sending EAP_NAK");
			return eap_payload_create_nak(in->get_identifier(in), 0, 0,
										  in->is_expanded(in));
		}
	}

	type = this->method->get_type(this->method, &vendor);

	if (this->method->process(this->method, in, &out) == NEED_MORE)
	{	/* client methods should never return SUCCESS */
		return out;
	}

	if (vendor)
	{
		DBG1(DBG_IKE, "vendor specific EAP method %d-%d failed", type, vendor);
	}
	else
	{
		DBG1(DBG_IKE, "%N method failed", eap_type_names, type);
	}
	return NULL;
}

/**
 * Verify AUTH payload
 */
static bool verify_auth(private_eap_authenticator_t *this, message_t *message,
						chunk_t nonce, chunk_t init)
{
	auth_payload_t *auth_payload;
	chunk_t auth_data, recv_auth_data;
	identification_t *other_id;
	auth_cfg_t *auth;
	keymat_v2_t *keymat;

	auth_payload = (auth_payload_t*)message->get_payload(message,
														 PLV2_AUTH);
	if (!auth_payload)
	{
		DBG1(DBG_IKE, "AUTH payload missing");
		return FALSE;
	}
	other_id = this->ike_sa->get_other_id(this->ike_sa);
	keymat = (keymat_v2_t*)this->ike_sa->get_keymat(this->ike_sa);
	if (!keymat->get_psk_sig(keymat, TRUE, init, nonce,
							 this->msk, other_id, this->reserved, &auth_data))
	{
		return FALSE;
	}
	recv_auth_data = auth_payload->get_data(auth_payload);
	if (!auth_data.len || !chunk_equals(auth_data, recv_auth_data))
	{
		DBG1(DBG_IKE, "verification of AUTH payload with%s EAP MSK failed",
			 this->msk.ptr ? "" : "out");
		chunk_free(&auth_data);
		return FALSE;
	}
	chunk_free(&auth_data);

	DBG1(DBG_IKE, "authentication of '%Y' with %N successful",
		 other_id, auth_class_names, AUTH_CLASS_EAP);
	this->auth_complete = TRUE;
	auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_EAP);
	return TRUE;
}

/**
 * Build AUTH payload
 */
static bool build_auth(private_eap_authenticator_t *this, message_t *message,
					   chunk_t nonce, chunk_t init)
{
	auth_payload_t *auth_payload;
	identification_t *my_id;
	chunk_t auth_data;
	keymat_v2_t *keymat;

	my_id = this->ike_sa->get_my_id(this->ike_sa);
	keymat = (keymat_v2_t*)this->ike_sa->get_keymat(this->ike_sa);

	DBG1(DBG_IKE, "authentication of '%Y' (myself) with %N",
		 my_id, auth_class_names, AUTH_CLASS_EAP);

	if (!keymat->get_psk_sig(keymat, FALSE, init, nonce,
							this->msk, my_id, this->reserved, &auth_data))
	{
		return FALSE;
	}
	auth_payload = auth_payload_create();
	auth_payload->set_auth_method(auth_payload, AUTH_PSK);
	auth_payload->set_data(auth_payload, auth_data);
	message->add_payload(message, (payload_t*)auth_payload);
	chunk_free(&auth_data);
	return TRUE;
}

METHOD(authenticator_t, process_server, status_t,
	private_eap_authenticator_t *this, message_t *message)
{
	eap_payload_t *eap_payload;

	if (this->eap_complete)
	{
		if (!verify_auth(this, message, this->sent_nonce, this->received_init))
		{
			return FAILED;
		}
		return NEED_MORE;
	}

	if (!this->method)
	{
		this->eap_payload = server_initiate_eap(this, TRUE);
	}
	else
	{
		eap_payload = (eap_payload_t*)message->get_payload(message,
													PLV2_EAP);
		if (!eap_payload)
		{
			return FAILED;
		}
		this->eap_payload = server_process_eap(this, eap_payload);
	}
	return NEED_MORE;
}

METHOD(authenticator_t, build_server, status_t,
	private_eap_authenticator_t *this, message_t *message)
{
	if (this->eap_payload)
	{
		eap_code_t code;

		code = this->eap_payload->get_code(this->eap_payload);
		message->add_payload(message, (payload_t*)this->eap_payload);
		this->eap_payload = NULL;
		if (code == EAP_FAILURE)
		{
			return FAILED;
		}
		return NEED_MORE;
	}
	if (this->eap_complete && this->auth_complete &&
		build_auth(this, message, this->received_nonce, this->sent_init))
	{
		return SUCCESS;
	}
	return FAILED;
}

METHOD(authenticator_t, process_client, status_t,
	private_eap_authenticator_t *this, message_t *message)
{
	eap_payload_t *eap_payload;

	if (this->eap_complete)
	{
		if (!verify_auth(this, message, this->sent_nonce, this->received_init))
		{
			return FAILED;
		}
		if (this->require_mutual && !this->method->is_mutual(this->method))
		{	/* we require mutual authentication due to EAP-only */
			u_int32_t vendor;

			DBG1(DBG_IKE, "EAP-only authentication requires a mutual and "
				 "MSK deriving EAP method, but %N is not",
				 eap_type_names, this->method->get_type(this->method, &vendor));
			return FAILED;
		}
		return SUCCESS;
	}

	eap_payload = (eap_payload_t*)message->get_payload(message,
													PLV2_EAP);
	if (eap_payload)
	{
		switch (eap_payload->get_code(eap_payload))
		{
			case EAP_REQUEST:
			{
				this->eap_payload = client_process_eap(this, eap_payload);
				if (this->eap_payload)
				{
					return NEED_MORE;
				}
				return FAILED;
			}
			case EAP_SUCCESS:
			{
				eap_type_t type;
				u_int32_t vendor;
				auth_cfg_t *cfg;

				if (this->method->get_msk(this->method, &this->msk) == SUCCESS)
				{
					this->msk = chunk_clone(this->msk);
				}
				type = this->method->get_type(this->method, &vendor);
				if (vendor)
				{
					DBG1(DBG_IKE, "EAP vendor specific method %d-%d succeeded, "
						 "%sMSK established", type, vendor,
						 this->msk.ptr ? "" : "no ");
				}
				else
				{
					DBG1(DBG_IKE, "EAP method %N succeeded, %sMSK established",
						 eap_type_names, type, this->msk.ptr ? "" : "no ");
				}
				cfg = this->ike_sa->get_auth_cfg(this->ike_sa, TRUE);
				cfg->add(cfg, AUTH_RULE_EAP_TYPE, type);
				if (vendor)
				{
					cfg->add(cfg, AUTH_RULE_EAP_VENDOR, vendor);
				}
				this->eap_complete = TRUE;
				return NEED_MORE;
			}
			case EAP_FAILURE:
			default:
			{
				DBG1(DBG_IKE, "received %N, EAP authentication failed",
					 eap_code_names, eap_payload->get_code(eap_payload));
				return FAILED;
			}
		}
	}
	return FAILED;
}

METHOD(authenticator_t, build_client, status_t,
	private_eap_authenticator_t *this, message_t *message)
{
	if (this->eap_payload)
	{
		message->add_payload(message, (payload_t*)this->eap_payload);
		this->eap_payload = NULL;
		return NEED_MORE;
	}
	if (this->eap_complete &&
		build_auth(this, message, this->received_nonce, this->sent_init))
	{
		return NEED_MORE;
	}
	return NEED_MORE;
}

METHOD(authenticator_t, is_mutual, bool,
	private_eap_authenticator_t *this)
{
	if (this->method)
	{
		u_int32_t vendor;

		if (this->method->get_type(this->method, &vendor) != EAP_IDENTITY ||
			vendor != 0)
		{
			return this->method->is_mutual(this->method);
		}
	}
	/* we don't know yet, but insist on it after EAP is complete */
	this->require_mutual = TRUE;
	return TRUE;
}

METHOD(authenticator_t, destroy, void,
	private_eap_authenticator_t *this)
{
	DESTROY_IF(this->method);
	DESTROY_IF(this->eap_payload);
	DESTROY_IF(this->eap_identity);
	chunk_free(&this->msk);
	free(this);
}

/*
 * Described in header.
 */
eap_authenticator_t *eap_authenticator_create_builder(ike_sa_t *ike_sa,
									chunk_t received_nonce, chunk_t sent_nonce,
									chunk_t received_init, chunk_t sent_init,
									char reserved[3])
{
	private_eap_authenticator_t *this;

	INIT(this,
		.public = {
			.authenticator = {
				.build = _build_client,
				.process = _process_client,
				.is_mutual = _is_mutual,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.received_init = received_init,
		.received_nonce = received_nonce,
		.sent_init = sent_init,
		.sent_nonce = sent_nonce,
	);
	memcpy(this->reserved, reserved, sizeof(this->reserved));

	return &this->public;
}

/*
 * Described in header.
 */
eap_authenticator_t *eap_authenticator_create_verifier(ike_sa_t *ike_sa,
									chunk_t received_nonce, chunk_t sent_nonce,
									chunk_t received_init, chunk_t sent_init,
									char reserved[3])
{
	private_eap_authenticator_t *this;

	INIT(this,
		.public = {
			.authenticator = {
				.build = _build_server,
				.process = _process_server,
				.is_mutual = _is_mutual,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.received_init = received_init,
		.received_nonce = received_nonce,
		.sent_init = sent_init,
		.sent_nonce = sent_nonce,
	);
	memcpy(this->reserved, reserved, sizeof(this->reserved));

	return &this->public;
}
