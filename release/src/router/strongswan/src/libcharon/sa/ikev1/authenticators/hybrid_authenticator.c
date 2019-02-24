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

#include "hybrid_authenticator.h"

#include <daemon.h>
#include <sa/ikev1/authenticators/psk_v1_authenticator.h>

typedef struct private_hybrid_authenticator_t private_hybrid_authenticator_t;

/**
 * Private data of an hybrid_authenticator_t object.
 */
struct private_hybrid_authenticator_t {

	/**
	 * Public authenticator_t interface.
	 */
	hybrid_authenticator_t public;

	/**
	 * Public key authenticator
	 */
	authenticator_t *sig;

	/**
	 * HASH payload authenticator without credentials
	 */
	authenticator_t *hash;
};

METHOD(authenticator_t, build_i, status_t,
	private_hybrid_authenticator_t *this, message_t *message)
{
	return this->hash->build(this->hash, message);
}

METHOD(authenticator_t, process_r, status_t,
	private_hybrid_authenticator_t *this, message_t *message)
{
	return this->hash->process(this->hash, message);
}

METHOD(authenticator_t, build_r, status_t,
	private_hybrid_authenticator_t *this, message_t *message)
{
	return this->sig->build(this->sig, message);
}

METHOD(authenticator_t, process_i, status_t,
	private_hybrid_authenticator_t *this, message_t *message)
{
	return this->sig->process(this->sig, message);
}

METHOD(authenticator_t, destroy, void,
	private_hybrid_authenticator_t *this)
{
	DESTROY_IF(this->hash);
	DESTROY_IF(this->sig);
	free(this);
}

/*
 * Described in header.
 */
hybrid_authenticator_t *hybrid_authenticator_create(ike_sa_t *ike_sa,
										bool initiator, diffie_hellman_t *dh,
										chunk_t dh_value, chunk_t sa_payload,
										chunk_t id_payload)
{
	private_hybrid_authenticator_t *this;

	INIT(this,
		.public = {
			.authenticator = {
				.is_mutual = (void*)return_false,
				.destroy = _destroy,
			},
		},
		.hash = (authenticator_t*)psk_v1_authenticator_create(ike_sa, initiator,
						dh, dh_value, sa_payload, id_payload, TRUE),
		.sig = authenticator_create_v1(ike_sa, initiator, AUTH_RSA, dh,
						dh_value, sa_payload, chunk_clone(id_payload)),
	);
	if (!this->sig || !this->hash)
	{
		destroy(this);
		return NULL;
	}
	if (initiator)
	{
		this->public.authenticator.build = _build_i;
		this->public.authenticator.process = _process_i;
	}
	else
	{
		this->public.authenticator.build = _build_r;
		this->public.authenticator.process = _process_r;
	}
	return &this->public;
}
