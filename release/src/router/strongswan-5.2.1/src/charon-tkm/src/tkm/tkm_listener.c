/*
 * Copyrigth (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <daemon.h>
#include <encoding/payloads/auth_payload.h>
#include <utils/chunk.h>
#include <tkm/types.h>
#include <tkm/constants.h>
#include <tkm/client.h>

#include "tkm.h"
#include "tkm_listener.h"
#include "tkm_keymat.h"
#include "tkm_utils.h"

typedef struct private_tkm_listener_t private_tkm_listener_t;

/**
 * Private data of a tkm_listener_t object.
 */
struct private_tkm_listener_t {

	/**
	 * Public tkm_listener_t interface.
	 */
	tkm_listener_t public;

};

/**
 * Return id of remote identity.
 *
 * TODO: Replace this with the lookup for the remote identitiy id.
 *
 * Currently the reqid of the first child SA in peer config of IKE SA is
 * returned. Might choose wrong reqid if IKE SA has multiple child configs
 * with different reqids.
 *
 * @param peer_cfg	Remote peer config
 * @return			remote identity id if found, 0 otherwise
 */
static ri_id_type get_remote_identity_id(peer_cfg_t *peer)
{
	ri_id_type remote_id = 0;
	child_cfg_t *child;
	enumerator_t* children;

	children = peer->create_child_cfg_enumerator(peer);

	/* pick the reqid of the first child, no need to enumerate all children. */
	children->enumerate(children, &child);
	remote_id = child->get_reqid(child);
	children->destroy(children);

	return remote_id;
}

/**
 * Build a TKM certificate chain context with given cc id.
 *
 * @param ike_sa	IKE SA containing auth config to build certificate chain from
 * @param cc_id		Certificate chain ID
 * @return			TRUE if certificate chain was built successfully,
 *					FALSE otherwise
 */
static bool build_cert_chain(const ike_sa_t * const ike_sa, cc_id_type cc_id)
{
	auth_cfg_t *auth;
	certificate_t *cert;
	enumerator_t *rounds;

	DBG1(DBG_IKE, "building certificate chain context %llu for IKE SA %s",
		 cc_id, ike_sa->get_name((ike_sa_t *)ike_sa));

	rounds = ike_sa->create_auth_cfg_enumerator((ike_sa_t *)ike_sa, FALSE);
	while (rounds->enumerate(rounds, &auth))
	{
		cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
		if (cert)
		{
			chunk_t enc_user_cert;
			ri_id_type ri_id;
			certificate_type user_cert;
			auth_rule_t rule;
			enumerator_t *enumerator;

			/* set user certificate */
			if (!cert->get_encoding(cert, CERT_ASN1_DER, &enc_user_cert))
			{
				DBG1(DBG_IKE, "unable to extract encoded user certificate");
				rounds->destroy(rounds);
				return FALSE;
			}

			ri_id = get_remote_identity_id(ike_sa->get_peer_cfg((ike_sa_t *)ike_sa));
			chunk_to_sequence(&enc_user_cert, &user_cert, sizeof(certificate_type));
			chunk_free(&enc_user_cert);
			if (ike_cc_set_user_certificate(cc_id, ri_id, 1, user_cert) != TKM_OK)
			{
				DBG1(DBG_IKE, "error setting user certificate of cert chain"
					 " (cc_id: %llu)", cc_id);
				rounds->destroy(rounds);
				return FALSE;
			}

			/* process intermediate CA certificates */
			enumerator = auth->create_enumerator(auth);
			while (enumerator->enumerate(enumerator, &rule, &cert))
			{
				if (rule == AUTH_RULE_IM_CERT)
				{
					chunk_t enc_im_cert;
					certificate_type im_cert;

					if (!cert->get_encoding(cert, CERT_ASN1_DER, &enc_im_cert))
					{
						DBG1(DBG_IKE, "unable to extract encoded intermediate CA"
							 " certificate");
						rounds->destroy(rounds);
						enumerator->destroy(enumerator);
						return FALSE;
					}

					chunk_to_sequence(&enc_im_cert, &im_cert,
									  sizeof(certificate_type));
					chunk_free(&enc_im_cert);
					if (ike_cc_add_certificate(cc_id, 1, im_cert) != TKM_OK)
					{
						DBG1(DBG_IKE, "error adding intermediate certificate to"
							 " cert chain (cc_id: %llu)", cc_id);
						rounds->destroy(rounds);
						enumerator->destroy(enumerator);
						return FALSE;
					}
				}
			}
			enumerator->destroy(enumerator);

			/* finally add CA certificate */
			cert = auth->get(auth, AUTH_RULE_CA_CERT);
			if (cert)
			{
				const ca_id_type ca_id = 1;
				certificate_type ca_cert;
				chunk_t enc_ca_cert;

				if (!cert->get_encoding(cert, CERT_ASN1_DER, &enc_ca_cert))
				{
					DBG1(DBG_IKE, "unable to extract encoded CA certificate");
					rounds->destroy(rounds);
					return FALSE;
				}

				chunk_to_sequence(&enc_ca_cert, &ca_cert,
								  sizeof(certificate_type));
				chunk_free(&enc_ca_cert);
				if (ike_cc_add_certificate(cc_id, 1, ca_cert) != TKM_OK)
				{
					DBG1(DBG_IKE, "error adding CA certificate to cert chain "
						 "(cc_id: %llu)", cc_id);
					rounds->destroy(rounds);
					return FALSE;
				}

				if (ike_cc_check_ca(cc_id, ca_id) != TKM_OK)
				{
					DBG1(DBG_IKE, "certificate chain (cc_id: %llu) not based on"
						 " trusted CA (ca_id: %llu)", cc_id, ca_id);
					rounds->destroy(rounds);
					return FALSE;
				}

				rounds->destroy(rounds);
				return TRUE;
			}
			else
			{
				DBG1(DBG_IKE, "no CA certificate");
			}
		}
		else
		{
			DBG1(DBG_IKE, "no subject certificate for remote peer");
		}
	}

	rounds->destroy(rounds);
	return FALSE;
}

METHOD(listener_t, alert, bool,
	private_tkm_listener_t *this, ike_sa_t *ike_sa,
	alert_t alert, va_list args)
{
	if (alert == ALERT_KEEP_ON_CHILD_SA_FAILURE)
	{
		tkm_keymat_t *keymat;
		isa_id_type isa_id;

		keymat = (tkm_keymat_t*)ike_sa->get_keymat(ike_sa);
		isa_id = keymat->get_isa_id(keymat);

		DBG1(DBG_IKE, "TKM alert listener called for ISA context %llu", isa_id);
		if (ike_isa_skip_create_first(isa_id) != TKM_OK)
		{
			DBG1(DBG_IKE, "Skip of first child SA creation failed for ISA "
				 "context %llu", isa_id);
		}
	}

	return TRUE;
}

METHOD(listener_t, authorize, bool,
	private_tkm_listener_t *this, ike_sa_t *ike_sa,
	bool final, bool *success)
{
	tkm_keymat_t *keymat;
	isa_id_type isa_id;
	cc_id_type cc_id;
	chunk_t *auth, *other_init_msg;
	signature_type signature;
	init_message_type init_msg;

	if (!final)
	{
		return TRUE;
	}

	keymat = (tkm_keymat_t*)ike_sa->get_keymat(ike_sa);
	isa_id = keymat->get_isa_id(keymat);
	DBG1(DBG_IKE, "TKM authorize listener called for ISA context %llu", isa_id);

	cc_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_CC);
	if (!cc_id)
	{
		DBG1(DBG_IKE, "unable to acquire CC context id");
		*success = FALSE;
		return TRUE;
	}
	if (!build_cert_chain(ike_sa, cc_id))
	{
		DBG1(DBG_IKE, "unable to build certificate chain");
		*success = FALSE;
		return TRUE;
	}

	auth = keymat->get_auth_payload(keymat);
	if (!auth->ptr)
	{
		DBG1(DBG_IKE, "no AUTHENTICATION data available");
		*success = FALSE;
	}

	other_init_msg = keymat->get_peer_init_msg(keymat);
	if (!other_init_msg->ptr)
	{
		DBG1(DBG_IKE, "no peer init message available");
		*success = FALSE;
	}

	chunk_to_sequence(auth, &signature, sizeof(signature_type));
	chunk_to_sequence(other_init_msg, &init_msg, sizeof(init_message_type));

	if (ike_isa_auth(isa_id, cc_id, init_msg, signature) != TKM_OK)
	{
		DBG1(DBG_IKE, "TKM based authentication failed"
			 " for ISA context %llu", isa_id);
		*success = FALSE;
	}
	else
	{
		DBG1(DBG_IKE, "TKM based authentication successful"
			 " for ISA context %llu", isa_id);
		*success = TRUE;
	}

	return TRUE;
}

METHOD(listener_t, message, bool,
	private_tkm_listener_t *this, ike_sa_t *ike_sa,
	message_t *message, bool incoming, bool plain)
{
	tkm_keymat_t *keymat;
	isa_id_type isa_id;
	auth_payload_t *auth_payload;

	if (!incoming || !plain || message->get_exchange_type(message) != IKE_AUTH)
	{
		return TRUE;
	}

	keymat = (tkm_keymat_t*)ike_sa->get_keymat(ike_sa);
	isa_id = keymat->get_isa_id(keymat);
	DBG1(DBG_IKE, "saving AUTHENTICATION payload for authorize hook"
	     " (ISA context %llu)", isa_id);

	auth_payload = (auth_payload_t*)message->get_payload(message,
														 PLV2_AUTH);
	if (auth_payload)
	{
		chunk_t auth_data;

		auth_data = auth_payload->get_data(auth_payload);
		keymat->set_auth_payload(keymat, &auth_data);
	}
	else
	{
		DBG1(DBG_IKE, "unable to extract AUTHENTICATION payload, authorize will"
			 " fail");
	}

	return TRUE;
}

METHOD(tkm_listener_t, destroy, void,
	private_tkm_listener_t *this)
{
	free(this);
}

/**
 * See header
 */
tkm_listener_t *tkm_listener_create()
{
	private_tkm_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.authorize = _authorize,
				.message = _message,
				.alert = _alert,
			},
			.destroy = _destroy,
		},
	);

	return &this->public;
}
