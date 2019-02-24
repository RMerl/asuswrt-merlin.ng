/*
 * Copyright (C) 2006-2009 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
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

#include <string.h>

#include "authenticator.h"

#include <sa/ikev2/authenticators/pubkey_authenticator.h>
#include <sa/ikev2/authenticators/psk_authenticator.h>
#include <sa/ikev2/authenticators/eap_authenticator.h>
#include <sa/ikev1/authenticators/psk_v1_authenticator.h>
#include <sa/ikev1/authenticators/pubkey_v1_authenticator.h>
#include <sa/ikev1/authenticators/hybrid_authenticator.h>
#include <encoding/payloads/auth_payload.h>


ENUM_BEGIN(auth_method_names, AUTH_RSA, AUTH_DSS,
	"RSA signature",
	"pre-shared key",
	"DSS signature");
ENUM_NEXT(auth_method_names, AUTH_ECDSA_256, AUTH_DS, AUTH_DSS,
	"ECDSA-256 signature",
	"ECDSA-384 signature",
	"ECDSA-521 signature",
	"secure password method",
	"NULL authentication",
	"digital signature");
ENUM_NEXT(auth_method_names, AUTH_XAUTH_INIT_PSK, AUTH_HYBRID_RESP_RSA, AUTH_DS,
	"XAuthInitPSK",
	"XAuthRespPSK",
	"XAuthInitRSA",
	"XauthRespRSA",
	"HybridInitRSA",
	"HybridRespRSA",
);
ENUM_END(auth_method_names, AUTH_HYBRID_RESP_RSA);

#ifdef USE_IKEV2

/**
 * Described in header.
 */
authenticator_t *authenticator_create_builder(ike_sa_t *ike_sa, auth_cfg_t *cfg,
									chunk_t received_nonce, chunk_t sent_nonce,
									chunk_t received_init, chunk_t sent_init,
									char reserved[3])
{
	switch ((uintptr_t)cfg->get(cfg, AUTH_RULE_AUTH_CLASS))
	{
		case AUTH_CLASS_ANY:
			/* defaults to PUBKEY */
		case AUTH_CLASS_PUBKEY:
			return (authenticator_t*)pubkey_authenticator_create_builder(ike_sa,
										received_nonce, sent_init, reserved);
		case AUTH_CLASS_PSK:
			return (authenticator_t*)psk_authenticator_create_builder(ike_sa,
										received_nonce, sent_init, reserved);
		case AUTH_CLASS_EAP:
			return (authenticator_t*)eap_authenticator_create_builder(ike_sa,
										received_nonce, sent_nonce,
										received_init, sent_init, reserved);
		default:
			return NULL;
	}
}

/**
 * Described in header.
 */
authenticator_t *authenticator_create_verifier(
									ike_sa_t *ike_sa, message_t *message,
									chunk_t received_nonce, chunk_t sent_nonce,
									chunk_t received_init, chunk_t sent_init,
									char reserved[3])
{
	auth_payload_t *auth_payload;

	auth_payload = (auth_payload_t*)message->get_payload(message, PLV2_AUTH);
	if (auth_payload == NULL)
	{
		return (authenticator_t*)eap_authenticator_create_verifier(ike_sa,
										received_nonce, sent_nonce,
										received_init, sent_init, reserved);
	}
	switch (auth_payload->get_auth_method(auth_payload))
	{
		case AUTH_RSA:
		case AUTH_ECDSA_256:
		case AUTH_ECDSA_384:
		case AUTH_ECDSA_521:
		case AUTH_DS:
			return (authenticator_t*)pubkey_authenticator_create_verifier(ike_sa,
										sent_nonce, received_init, reserved);
		case AUTH_PSK:
			return (authenticator_t*)psk_authenticator_create_verifier(ike_sa,
										sent_nonce, received_init, reserved);
		default:
			return NULL;
	}
}

#endif /* USE_IKEV2 */

#ifdef USE_IKEV1

/**
 * Described in header.
 */
authenticator_t *authenticator_create_v1(ike_sa_t *ike_sa, bool initiator,
								auth_method_t auth_method, diffie_hellman_t *dh,
								chunk_t dh_value, chunk_t sa_payload,
								chunk_t id_payload)
{
	switch (auth_method)
	{
		case AUTH_PSK:
		case AUTH_XAUTH_INIT_PSK:
		case AUTH_XAUTH_RESP_PSK:
			return (authenticator_t*)psk_v1_authenticator_create(ike_sa,
										initiator, dh, dh_value, sa_payload,
										id_payload, FALSE);
		case AUTH_RSA:
		case AUTH_XAUTH_INIT_RSA:
		case AUTH_XAUTH_RESP_RSA:
			return (authenticator_t*)pubkey_v1_authenticator_create(ike_sa,
										initiator, dh, dh_value, sa_payload,
										id_payload, KEY_RSA);
		case AUTH_ECDSA_256:
		case AUTH_ECDSA_384:
		case AUTH_ECDSA_521:
			return (authenticator_t*)pubkey_v1_authenticator_create(ike_sa,
										initiator, dh, dh_value, sa_payload,
										id_payload, KEY_ECDSA);
		case AUTH_HYBRID_INIT_RSA:
		case AUTH_HYBRID_RESP_RSA:
			return (authenticator_t*)hybrid_authenticator_create(ike_sa,
										initiator, dh, dh_value, sa_payload,
										id_payload);
		default:
			return NULL;
	}
}

#endif /* USE_IKEV1 */
