/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "psk_v1_authenticator.h"

#include <daemon.h>
#include <sa/ikev1/keymat_v1.h>
#include <encoding/payloads/hash_payload.h>

typedef struct private_psk_v1_authenticator_t private_psk_v1_authenticator_t;

/**
 * Private data of an psk_v1_authenticator_t object.
 */
struct private_psk_v1_authenticator_t {

	/**
	 * Public authenticator_t interface.
	 */
	psk_v1_authenticator_t public;

	/**
	 * Assigned IKE_SA
	 */
	ike_sa_t *ike_sa;

	/**
	 * TRUE if we are initiator
	 */
	bool initiator;

	/**
	 * DH key exchange
	 */
	diffie_hellman_t *dh;

	/**
	 * Others DH public value
	 */
	chunk_t dh_value;

	/**
	 * Encoded SA payload, without fixed header
	 */
	chunk_t sa_payload;

	/**
	 * Encoded ID payload, without fixed header
	 */
	chunk_t id_payload;

	/**
	 * Used for Hybrid authentication to build hash without PSK?
	 */
	bool hybrid;
};

METHOD(authenticator_t, build, status_t,
	private_psk_v1_authenticator_t *this, message_t *message)
{
	hash_payload_t *hash_payload;
	keymat_v1_t *keymat;
	chunk_t hash, dh;

	this->dh->get_my_public_value(this->dh, &dh);
	keymat = (keymat_v1_t*)this->ike_sa->get_keymat(this->ike_sa);
	if (!keymat->get_hash(keymat, this->initiator, dh, this->dh_value,
					this->ike_sa->get_id(this->ike_sa), this->sa_payload,
					this->id_payload, &hash))
	{
		free(dh.ptr);
		return FAILED;
	}
	free(dh.ptr);

	hash_payload = hash_payload_create(PLV1_HASH);
	hash_payload->set_hash(hash_payload, hash);
	message->add_payload(message, &hash_payload->payload_interface);
	free(hash.ptr);

	return SUCCESS;
}

METHOD(authenticator_t, process, status_t,
	private_psk_v1_authenticator_t *this, message_t *message)
{
	hash_payload_t *hash_payload;
	keymat_v1_t *keymat;
	chunk_t hash, dh;
	auth_cfg_t *auth;

	hash_payload = (hash_payload_t*)message->get_payload(message, PLV1_HASH);
	if (!hash_payload)
	{
		DBG1(DBG_IKE, "HASH payload missing in message");
		return FAILED;
	}

	this->dh->get_my_public_value(this->dh, &dh);
	keymat = (keymat_v1_t*)this->ike_sa->get_keymat(this->ike_sa);
	if (!keymat->get_hash(keymat, !this->initiator, this->dh_value, dh,
					this->ike_sa->get_id(this->ike_sa), this->sa_payload,
					this->id_payload, &hash))
	{
		free(dh.ptr);
		return FAILED;
	}
	free(dh.ptr);
	if (chunk_equals(hash, hash_payload->get_hash(hash_payload)))
	{
		free(hash.ptr);
		if (!this->hybrid)
		{
			auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);
			auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PSK);
		}
		return SUCCESS;
	}
	free(hash.ptr);
	DBG1(DBG_IKE, "calculated HASH does not match HASH payload");
	return FAILED;
}

METHOD(authenticator_t, destroy, void,
	private_psk_v1_authenticator_t *this)
{
	chunk_free(&this->id_payload);
	free(this);
}

/*
 * Described in header.
 */
psk_v1_authenticator_t *psk_v1_authenticator_create(ike_sa_t *ike_sa,
										bool initiator, diffie_hellman_t *dh,
										chunk_t dh_value, chunk_t sa_payload,
										chunk_t id_payload, bool hybrid)
{
	private_psk_v1_authenticator_t *this;

	INIT(this,
		.public = {
			.authenticator = {
				.build = _build,
				.process = _process,
				.is_mutual = (void*)return_false,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
		.dh = dh,
		.dh_value = dh_value,
		.sa_payload = sa_payload,
		.id_payload = id_payload,
		.hybrid = hybrid,
	);

	return &this->public;
}
