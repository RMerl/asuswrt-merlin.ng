/*
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

#include "eap_tnc.h"

#include <tnc/tnc.h>
#include <tnc/tnccs/tnccs_manager.h>
#include <tls_eap.h>
#include <utils/debug.h>
#include <daemon.h>

#include <tncifimv.h>
#include <tncif_names.h>

/**
 * Maximum size of an EAP-TNC message
 */
#define EAP_TNC_MAX_MESSAGE_LEN 65535

/**
 * Maximum number of EAP-TNC messages allowed
 */
#define EAP_TNC_MAX_MESSAGE_COUNT 10

typedef struct private_eap_tnc_t private_eap_tnc_t;

/**
 * Private data of an eap_tnc_t object.
 */
struct private_eap_tnc_t {

	/**
	 * Public authenticator_t interface.
	 */
	eap_tnc_t public;

	/**
	 * Inner EAP authentication type
	 */
	eap_type_t type;

	/**
	 * Outer EAP authentication type
	 */
	eap_type_t auth_type;

	/**
	 * TLS stack, wrapped by EAP helper
	 */
	tls_eap_t *tls_eap;

	/**
	 * TNCCS instance running over EAP-TNC
	 */
	tnccs_t *tnccs;

};

/**
 * Callback function to get recommendation from TNCCS connection
 */
static bool enforce_recommendation(TNC_IMV_Action_Recommendation rec,
								   TNC_IMV_Evaluation_Result eval)
{
	char *group;
	identification_t *id;
	ike_sa_t *ike_sa;
	auth_cfg_t *auth;
	bool no_access = FALSE;

	DBG1(DBG_TNC, "final recommendation is '%N' and evaluation is '%N'",
		 TNC_IMV_Action_Recommendation_names, rec,
		 TNC_IMV_Evaluation_Result_names, eval);

	switch (rec)
	{
		case TNC_IMV_ACTION_RECOMMENDATION_ALLOW:
			group = "allow";
			break;
		case TNC_IMV_ACTION_RECOMMENDATION_ISOLATE:
			group = "isolate";
			break;
		case TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS:
		case TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION:
		default:
			group = "no access";
			no_access = TRUE;
			break;
	}

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		DBG1(DBG_TNC, "policy enforcement point did not find IKE_SA");
		return FALSE;
	}

	id = ike_sa->get_other_id(ike_sa);
	DBG0(DBG_TNC, "policy enforced on peer '%Y' is '%s'", id, group);

	if (no_access)
	{
		return FALSE;
	}
	else
	{
		auth = ike_sa->get_auth_cfg(ike_sa, FALSE);
		id = identification_create_from_string(group);
		auth->add(auth, AUTH_RULE_GROUP, id);
		DBG1(DBG_TNC, "policy enforcement point added group membership '%s'",
			 group);
	}
	return TRUE;
}

METHOD(eap_method_t, initiate, status_t,
	private_eap_tnc_t *this, eap_payload_t **out)
{
	chunk_t data;
	uint32_t auth_type;

	/* Determine TNC Client Authentication Type */
	switch (this->auth_type)
	{
		case EAP_TLS:
		case EAP_TTLS:
		case EAP_PEAP:
			auth_type = TNC_AUTH_X509_CERT;
			break;
		case EAP_MD5:
		case EAP_MSCHAPV2:
		case EAP_GTC:
		case EAP_OTP:
			auth_type = TNC_AUTH_PASSWORD;
			break;
		case EAP_SIM:
		case EAP_AKA:
			auth_type = TNC_AUTH_SIM;
			break;
		default:
			auth_type = TNC_AUTH_UNKNOWN;
	}
	this->tnccs->set_auth_type(this->tnccs, auth_type);

	if (this->tls_eap->initiate(this->tls_eap, &data) == NEED_MORE)
	{
		*out = eap_payload_create_data(data);
		free(data.ptr);
		return NEED_MORE;
	}
	return FAILED;
}

METHOD(eap_method_t, process, status_t,
	private_eap_tnc_t *this, eap_payload_t *in, eap_payload_t **out)
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
	private_eap_tnc_t *this, uint32_t *vendor)
{
	*vendor = 0;
	return this->type;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_tnc_t *this, chunk_t *msk)
{
	*msk = this->tls_eap->get_msk(this->tls_eap);
	if (msk->len)
	{
		return SUCCESS;
	}
	return FAILED;
}

METHOD(eap_method_t, get_identifier, uint8_t,
	private_eap_tnc_t *this)
{
	return this->tls_eap->get_identifier(this->tls_eap);
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_tnc_t *this, uint8_t identifier)
{
	this->tls_eap->set_identifier(this->tls_eap, identifier);
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_tnc_t *this)
{
	return FALSE;
}

METHOD(eap_method_t, destroy, void,
	private_eap_tnc_t *this)
{
	chunk_t pdp_server;
	uint16_t pdp_port;
	tls_t *tls;

	pdp_server = this->tnccs->get_pdp_server(this->tnccs, &pdp_port);
	if (pdp_server.len)
	{
		DBG2(DBG_TNC, "TODO: setup PT-TLS connection to %.*s:%u",
			 pdp_server.len, pdp_server.ptr, pdp_port);
	}
	tls = &this->tnccs->tls;
	tls->destroy(tls);
	this->tls_eap->destroy(this->tls_eap);
	free(this);
}

METHOD(eap_inner_method_t, get_auth_type, eap_type_t,
	private_eap_tnc_t *this)
{
	return this->auth_type;
}

METHOD(eap_inner_method_t, set_auth_type, void,
	private_eap_tnc_t *this, eap_type_t type)
{
	this->auth_type = type;
}

/**
 * Generic private constructor
 */
static eap_tnc_t *eap_tnc_create(identification_t *server,
								 identification_t *peer, bool is_server,
								 eap_type_t type)
{
	private_eap_tnc_t *this;
	int max_msg_count;
	char* protocol;
	ike_sa_t *ike_sa;
	host_t *server_ip, *peer_ip;
	tnccs_t *tnccs;
	tnccs_type_t tnccs_type;

	INIT(this,
		.public = {
			.eap_inner_method = {
				.eap_method = {
					.initiate = _initiate,
					.process = _process,
					.get_type = _get_type,
					.is_mutual = _is_mutual,
					.get_msk = _get_msk,
					.get_identifier = _get_identifier,
					.set_identifier = _set_identifier,
					.destroy = _destroy,
				},
				.get_auth_type = _get_auth_type,
				.set_auth_type = _set_auth_type,
			},
		},
		.type = type,
	);

	max_msg_count = lib->settings->get_int(lib->settings,
						"%s.plugins.eap-tnc.max_message_count",
						EAP_TNC_MAX_MESSAGE_COUNT, lib->ns);
	protocol = lib->settings->get_str(lib->settings,
						"%s.plugins.eap-tnc.protocol", "tnccs-2.0", lib->ns);
	if (strcaseeq(protocol, "tnccs-2.0"))
	{
		tnccs_type = TNCCS_2_0;
	}
	else if (strcaseeq(protocol, "tnccs-1.1"))
	{
		tnccs_type = TNCCS_1_1;
	}
	else if (strcaseeq(protocol, "tnccs-dynamic") && is_server)
	{
		tnccs_type = TNCCS_DYNAMIC;
	}
	else
	{
		DBG1(DBG_TNC, "TNCCS protocol '%s' not supported", protocol);
		free(this);
		return NULL;
	}

	/* Determine IP addresses of server and peer */
	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		DBG1(DBG_TNC, "%N constructor did not find IKE_SA",
					   eap_type_names, type);
		free(this);
		return NULL;
	}
	if (is_server)
	{
		server_ip = ike_sa->get_my_host(ike_sa);
		peer_ip = ike_sa->get_other_host(ike_sa);
	}
	else
	{
		peer_ip = ike_sa->get_my_host(ike_sa);
		server_ip = ike_sa->get_other_host(ike_sa);
	}

	tnccs = tnc->tnccs->create_instance(tnc->tnccs, tnccs_type,
						is_server, server, peer, server_ip, peer_ip,
						(type == EAP_TNC) ? TNC_IFT_EAP_1_1 : TNC_IFT_EAP_2_0,
						enforce_recommendation);
	if (!tnccs)
	{
		DBG1(DBG_TNC, "TNCCS protocol '%s' not enabled", protocol);
		free(this);
		return NULL;
	}
	if (!is_server)
	{
		tnccs->set_auth_type(tnccs, TNC_AUTH_X509_CERT);
	}
	this->tnccs = tnccs->get_ref(tnccs);
	this->tls_eap = tls_eap_create(type, &tnccs->tls,
								   EAP_TNC_MAX_MESSAGE_LEN,
								   max_msg_count, FALSE);
	if (!this->tls_eap)
	{
		free(this);
		return NULL;
	}
	return &this->public;
}

eap_tnc_t *eap_tnc_create_server(identification_t *server,
								 identification_t *peer)
{
	return eap_tnc_create(server, peer, TRUE, EAP_TNC);
}

eap_tnc_t *eap_tnc_create_peer(identification_t *server,
							   identification_t *peer)
{
	return eap_tnc_create(server, peer, FALSE, EAP_TNC);
}

eap_tnc_t *eap_tnc_pt_create_server(identification_t *server,
								 identification_t *peer)
{
	return eap_tnc_create(server, peer, TRUE, EAP_PT_EAP);
}

eap_tnc_t *eap_tnc_pt_create_peer(identification_t *server,
							   identification_t *peer)
{
	return eap_tnc_create(server, peer, FALSE, EAP_PT_EAP);
}
