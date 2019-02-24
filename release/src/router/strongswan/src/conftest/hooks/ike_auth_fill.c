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

#include <time.h>
#include <netinet/udp.h>

#include <encoding/payloads/cert_payload.h>
#include <encoding/payloads/encrypted_payload.h>

typedef struct private_ike_auth_fill_t private_ike_auth_fill_t;

/**
 * Private data of an ike_auth_fill_t object.
 */
struct private_ike_auth_fill_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;

	/**
	 * Alter requests or responses?
	 */
	bool req;

	/**
	 * ID of message to alter.
	 */
	int id;

	/**
	 * Number of bytes to fill IKE_AUTH up
	 */
	int bytes;
};

/** size of non ESP-Marker */
#define NON_ESP_MARKER_LEN 4
/** length of fixed encryption payload header */
#define ENCRYPTION_PAYLOAD_HEADER_LENGTH 4
/** length of fixed cert payload header */
#define CERT_PAYLOAD_HEADER_LENGTH 5
/**
 * Calculate packet size on wire (without ethernet/IP header)
 */
static size_t calculate_wire_size(message_t *message, ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	payload_t *payload;
	size_t size = 0;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		size += payload->get_length(payload);
	}
	enumerator->destroy(enumerator);

	if (message->get_exchange_type(message) != IKE_SA_INIT)
	{
		keymat_t *keymat;
		aead_t *aead;
		size_t  bs;

		keymat = ike_sa->get_keymat(ike_sa);
		aead = keymat->get_aead(keymat, FALSE);
		if (aead)
		{
			bs = aead->get_block_size(aead);
			size += ENCRYPTION_PAYLOAD_HEADER_LENGTH + NON_ESP_MARKER_LEN +
				aead->get_icv_size(aead) + aead->get_iv_size(aead) +
				(bs - (size % bs));
		}
	}
	return sizeof(struct udphdr) + IKE_HEADER_LENGTH + size;
}

METHOD(listener_t, message, bool,
	private_ike_auth_fill_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		cert_payload_t *pld;
		size_t size, diff;
		chunk_t data;

		size = calculate_wire_size(message, ike_sa);
		if (size < this->bytes - CERT_PAYLOAD_HEADER_LENGTH)
		{
			diff = this->bytes - size - CERT_PAYLOAD_HEADER_LENGTH;
			data = chunk_alloc(diff);
			memset(data.ptr, 0x12, data.len);
			pld = cert_payload_create_custom(PLV2_CERTIFICATE, 201, data);
			message->add_payload(message, &pld->payload_interface);
			DBG1(DBG_CFG, "inserting %d dummy bytes certificate payload", diff);
		}
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_ike_auth_fill_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *ike_auth_fill_hook_create(char *name)
{
	private_ike_auth_fill_t *this;

	INIT(this,
		.hook = {
			.listener = {
				.message = _message,
			},
			.destroy = _destroy,
		},
		.req = conftest->test->get_bool(conftest->test,
										"hooks.%s.request", TRUE, name),
		.id = conftest->test->get_int(conftest->test,
										"hooks.%s.id", 1, name),
		.bytes = conftest->test->get_int(conftest->test,
										"hooks.%s.bytes", 0, name),
	);

	return &this->hook;
}
