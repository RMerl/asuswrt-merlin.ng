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

#include <encoding/payloads/unknown_payload.h>

typedef struct private_force_cookie_t private_force_cookie_t;

/**
 * Private data of an force_cookie_t object.
 */
struct private_force_cookie_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;
};

METHOD(listener_t, message, bool,
	private_force_cookie_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (incoming && plain && message->get_request(message) &&
		message->get_exchange_type(message) == IKE_SA_INIT)
	{
		enumerator_t *enumerator;
		bool has_cookie = FALSE;
		payload_t *payload;

		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (payload->get_type(payload) == PLV2_NOTIFY)
			{
				notify_payload_t *notify = (notify_payload_t*)payload;
				chunk_t data;

				if (notify->get_notify_type(notify) == COOKIE)
				{
					data = notify->get_notification_data(notify);
					DBG1(DBG_CFG, "received COOKIE: %#B", &data);
					has_cookie = TRUE;
					break;
				}
			}
		}
		enumerator->destroy(enumerator);
		if (!has_cookie)
		{
			message_t *response;
			host_t *src, *dst;
			packet_t *packet;
			ike_sa_id_t *ike_sa_id;
			chunk_t data = chunk_from_thing("COOKIE test data");

			DBG1(DBG_CFG, "sending COOKIE: %#B", &data);
			response = message_create(IKEV2_MAJOR_VERSION, IKEV2_MINOR_VERSION);
			dst = message->get_source(message);
			src = message->get_destination(message);
			response->set_source(response, src->clone(src));
			response->set_destination(response, dst->clone(dst));
			response->set_exchange_type(response, IKE_SA_INIT);
			response->set_request(response, FALSE);
			response->set_message_id(response, 0);
			ike_sa_id = message->get_ike_sa_id(message);
			ike_sa_id->switch_initiator(ike_sa_id);
			response->set_ike_sa_id(response, ike_sa_id);
			response->add_notify(response, FALSE, COOKIE, data);
			if (response->generate(response, NULL, &packet) == SUCCESS)
			{
				charon->sender->send(charon->sender, packet);
				response->destroy(response);
			}
			message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
		}
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_force_cookie_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *force_cookie_hook_create(char *name)
{
	private_force_cookie_t *this;

	INIT(this,
		.hook = {
			.listener = {
				.message = _message,
			},
			.destroy = _destroy,
		},
	);

	return &this->hook;
}
