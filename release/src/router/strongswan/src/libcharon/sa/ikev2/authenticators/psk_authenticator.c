/*
 * Copyright (C) 2018 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "psk_authenticator.h"

#include <daemon.h>
#include <encoding/payloads/auth_payload.h>
#include <sa/ikev2/keymat_v2.h>

typedef struct private_psk_authenticator_t private_psk_authenticator_t;

/**
 * Private data of an psk_authenticator_t object.
 */
struct private_psk_authenticator_t {

	/**
	 * Public authenticator_t interface.
	 */
	psk_authenticator_t public;

	/**
	 * Assigned IKE_SA
	 */
	ike_sa_t *ike_sa;

	/**
	 * nonce to include in AUTH calculation
	 */
	chunk_t nonce;

	/**
	 * IKE_SA_INIT message data to include in AUTH calculation
	 */
	chunk_t ike_sa_init;

	/**
	 * Reserved bytes of ID payload
	 */
	char reserved[3];

	/**
	 * PPK to use
	 */
	chunk_t ppk;

	/**
	 * Add a NO_PPK_AUTH notify
	 */
	bool no_ppk_auth;
};

METHOD(authenticator_t, build, status_t,
	private_psk_authenticator_t *this, message_t *message)
{
	identification_t *my_id, *other_id;
	auth_payload_t *auth_payload;
	shared_key_t *key;
	chunk_t auth_data;
	keymat_v2_t *keymat;

	keymat = (keymat_v2_t*)this->ike_sa->get_keymat(this->ike_sa);
	my_id = this->ike_sa->get_my_id(this->ike_sa);
	other_id = this->ike_sa->get_other_id(this->ike_sa);
	DBG1(DBG_IKE, "authentication of '%Y' (myself) with %N",
		 my_id, auth_method_names, AUTH_PSK);
	key = lib->credmgr->get_shared(lib->credmgr, SHARED_IKE, my_id, other_id);
	if (!key)
	{
		DBG1(DBG_IKE, "no shared key found for '%Y' - '%Y'", my_id, other_id);
		return NOT_FOUND;
	}
	if (!keymat->get_psk_sig(keymat, FALSE, this->ike_sa_init, this->nonce,
							 key->get_key(key), this->ppk, my_id,
							 this->reserved, &auth_data))
	{
		key->destroy(key);
		return FAILED;
	}

	DBG2(DBG_IKE, "successfully created shared key MAC");
	auth_payload = auth_payload_create();
	auth_payload->set_auth_method(auth_payload, AUTH_PSK);
	auth_payload->set_data(auth_payload, auth_data);
	chunk_free(&auth_data);
	message->add_payload(message, (payload_t*)auth_payload);

	if (this->no_ppk_auth)
	{
		if (!keymat->get_psk_sig(keymat, FALSE, this->ike_sa_init, this->nonce,
							 key->get_key(key), chunk_empty, my_id,
							 this->reserved, &auth_data))
		{
			DBG1(DBG_IKE, "failed adding NO_PPK_AUTH notify");
			key->destroy(key);
			return SUCCESS;
		}
		DBG2(DBG_IKE, "successfully created shared key MAC without PPK");
		message->add_notify(message, FALSE, NO_PPK_AUTH, auth_data);
		chunk_free(&auth_data);
	}
	key->destroy(key);
	return SUCCESS;
}

METHOD(authenticator_t, process, status_t,
	private_psk_authenticator_t *this, message_t *message)
{
	chunk_t auth_data, recv_auth_data;
	identification_t *my_id, *other_id;
	auth_payload_t *auth_payload;
	notify_payload_t *notify;
	auth_cfg_t *auth;
	shared_key_t *key;
	enumerator_t *enumerator;
	bool authenticated = FALSE;
	int keys_found = 0;
	keymat_v2_t *keymat;

	auth_payload = (auth_payload_t*)message->get_payload(message, PLV2_AUTH);
	if (!auth_payload)
	{
		return FAILED;
	}
	recv_auth_data = auth_payload->get_data(auth_payload);

	if (this->ike_sa->supports_extension(this->ike_sa, EXT_PPK) &&
		!this->ppk.ptr)
	{	/* look for a NO_PPK_AUTH notify if we have no PPK */
		notify = message->get_notify(message, NO_PPK_AUTH);
		if (notify)
		{
			DBG1(DBG_IKE, "no PPK available, using NO_PPK_AUTH notify");
			recv_auth_data = notify->get_notification_data(notify);
		}
	}

	keymat = (keymat_v2_t*)this->ike_sa->get_keymat(this->ike_sa);
	my_id = this->ike_sa->get_my_id(this->ike_sa);
	other_id = this->ike_sa->get_other_id(this->ike_sa);
	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
												SHARED_IKE, my_id, other_id);
	while (!authenticated && enumerator->enumerate(enumerator, &key, NULL, NULL))
	{
		keys_found++;

		if (!keymat->get_psk_sig(keymat, TRUE, this->ike_sa_init, this->nonce,
								 key->get_key(key), this->ppk, other_id,
								 this->reserved, &auth_data))
		{
			continue;
		}
		if (auth_data.len && chunk_equals_const(auth_data, recv_auth_data))
		{
			DBG1(DBG_IKE, "authentication of '%Y' with %N successful",
				 other_id, auth_method_names, AUTH_PSK);
			authenticated = TRUE;
		}
		chunk_free(&auth_data);
	}
	enumerator->destroy(enumerator);

	if (!authenticated)
	{
		if (keys_found == 0)
		{
			DBG1(DBG_IKE, "no shared key found for '%Y' - '%Y'", my_id, other_id);
			return NOT_FOUND;
		}
		DBG1(DBG_IKE, "tried %d shared key%s for '%Y' - '%Y', but MAC mismatched",
			 keys_found, keys_found == 1 ? "" : "s", my_id, other_id);
		return FAILED;
	}

	auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PSK);
	return SUCCESS;
}

METHOD(authenticator_t, use_ppk, void,
	private_psk_authenticator_t *this, chunk_t ppk, bool no_ppk_auth)
{
	this->ppk = ppk;
	this->no_ppk_auth = no_ppk_auth;
}

METHOD(authenticator_t, destroy, void,
	private_psk_authenticator_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
psk_authenticator_t *psk_authenticator_create_builder(ike_sa_t *ike_sa,
									chunk_t received_nonce, chunk_t sent_init,
									char reserved[3])
{
	private_psk_authenticator_t *this;

	INIT(this,
		.public = {
			.authenticator = {
				.build = _build,
				.process = (void*)return_failed,
				.use_ppk = _use_ppk,
				.is_mutual = (void*)return_false,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.ike_sa_init = sent_init,
		.nonce = received_nonce,
	);
	memcpy(this->reserved, reserved, sizeof(this->reserved));

	return &this->public;
}

/*
 * Described in header.
 */
psk_authenticator_t *psk_authenticator_create_verifier(ike_sa_t *ike_sa,
									chunk_t sent_nonce, chunk_t received_init,
									char reserved[3])
{
	private_psk_authenticator_t *this;

	INIT(this,
		.public = {
			.authenticator = {
				.build = (void*)return_failed,
				.process = _process,
				.use_ppk = _use_ppk,
				.is_mutual = (void*)return_false,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.ike_sa_init = received_init,
		.nonce = sent_nonce,
	);
	memcpy(this->reserved, reserved, sizeof(this->reserved));

	return &this->public;
}
