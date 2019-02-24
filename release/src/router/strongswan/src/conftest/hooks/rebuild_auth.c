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
#include <encoding/generator.h>
#include <encoding/payloads/nonce_payload.h>
#include <encoding/payloads/auth_payload.h>
#include <encoding/payloads/id_payload.h>

typedef struct private_rebuild_auth_t private_rebuild_auth_t;

/**
 * Private data of an rebuild_auth_t object.
 */
struct private_rebuild_auth_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;

	/**
	 * Our IKE_SA_INIT data, required to rebuild AUTH
	 */
	chunk_t ike_init;

	/**
	 * Received NONCE, required to rebuild AUTH
	 */
	chunk_t nonce;

	/**
	 * ID to use for key lookup, if not from IDi
	 */
	identification_t *id;
};

/**
 * Rebuild our AUTH data
 */
static bool rebuild_auth(private_rebuild_auth_t *this, ike_sa_t *ike_sa,
						 message_t *message)
{
	enumerator_t *enumerator;
	chunk_t octets, auth_data;
	private_key_t *private;
	payload_t *payload;
	auth_payload_t *auth_payload;
	auth_method_t auth_method;
	signature_scheme_t scheme;
	keymat_v2_t *keymat;
	identification_t *id;
	char reserved[3];
	generator_t *generator;
	chunk_t data;
	uint32_t *lenpos;

	payload = message->get_payload(message,
					message->get_request(message) ? PLV2_ID_INITIATOR : PLV2_ID_RESPONDER);
	if (!payload)
	{
		DBG1(DBG_CFG, "ID payload not found to rebuild AUTH");
		return FALSE;
	}

	generator = generator_create();
	generator->generate_payload(generator, payload);
	data = generator->get_chunk(generator, &lenpos);
	if (data.len < 8)
	{
		DBG1(DBG_CFG, "ID payload invalid to rebuild AUTH");
		generator->destroy(generator);
		return FALSE;
	}
	memcpy(reserved, data.ptr + 5, 3);
	id = identification_create_from_encoding(data.ptr[4], chunk_skip(data, 8));
	generator->destroy(generator);

	private = lib->credmgr->get_private(lib->credmgr, KEY_ANY,
										this->id ?: id, NULL);
	if (private == NULL)
	{
		DBG1(DBG_CFG, "no private key found for '%Y' to rebuild AUTH",
			 this->id ?: id);
		id->destroy(id);
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
					id->destroy(id);
					return FALSE;
			}
			break;
		default:
			DBG1(DBG_CFG, "private key of type %N not supported",
					key_type_names, private->get_type(private));
			id->destroy(id);
			return FALSE;
	}
	keymat = (keymat_v2_t*)ike_sa->get_keymat(ike_sa);
	if (!keymat->get_auth_octets(keymat, FALSE, this->ike_init, this->nonce,
								 chunk_empty, id, reserved, &octets, NULL))
	{
		private->destroy(private);
		id->destroy(id);
		return FALSE;
	}
	if (!private->sign(private, scheme, NULL, octets, &auth_data))
	{
		chunk_free(&octets);
		private->destroy(private);
		id->destroy(id);
		return FALSE;
	}
	auth_payload = auth_payload_create();
	auth_payload->set_auth_method(auth_payload, auth_method);
	auth_payload->set_data(auth_payload, auth_data);
	chunk_free(&auth_data);
	chunk_free(&octets);
	private->destroy(private);

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV2_AUTH)
		{
			message->remove_payload_at(message, enumerator);
			payload->destroy(payload);
		}
	}
	enumerator->destroy(enumerator);

	message->add_payload(message, (payload_t*)auth_payload);
	DBG1(DBG_CFG, "rebuilding AUTH payload for '%Y' with %N",
		 id, auth_method_names, auth_method);
	id->destroy(id);
	return TRUE;
}

METHOD(listener_t, message, bool,
	private_rebuild_auth_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (plain)
	{
		if (!incoming && message->get_message_id(message) == 1)
		{
			rebuild_auth(this, ike_sa, message);
		}
		if (message->get_exchange_type(message) == IKE_SA_INIT)
		{
			if (incoming)
			{
				nonce_payload_t *nonce;

				nonce = (nonce_payload_t*)message->get_payload(message, PLV2_NONCE);
				if (nonce)
				{
					free(this->nonce.ptr);
					this->nonce = nonce->get_nonce(nonce);
				}
			}
			else
			{
				packet_t *packet;

				if (message->generate(message, NULL, &packet) == SUCCESS)
				{
					free(this->ike_init.ptr);
					this->ike_init = chunk_clone(packet->get_data(packet));
					packet->destroy(packet);
				}
			}
		}
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_rebuild_auth_t *this)
{
	free(this->ike_init.ptr);
	free(this->nonce.ptr);
	DESTROY_IF(this->id);
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *rebuild_auth_hook_create(char *name)
{
	private_rebuild_auth_t *this;
	char *id;

	INIT(this,
		.hook = {
			.listener = {
				.message = _message,
			},
			.destroy = _destroy,
		},
	);
	id = conftest->test->get_str(conftest->test, "hooks.%s.key", NULL, name);
	if (id)
	{
		this->id = identification_create_from_string(id);
	}

	return &this->hook;
}
