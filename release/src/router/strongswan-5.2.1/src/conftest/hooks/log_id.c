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

#include <encoding/payloads/id_payload.h>

typedef struct private_log_id_t private_log_id_t;

/**
 * Private data of an log_id_t object.
 */
struct private_log_id_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;
};

METHOD(listener_t, message, bool,
	private_log_id_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (incoming && plain)
	{
		enumerator_t *enumerator;
		payload_t *payload;
		id_payload_t *id_payload;
		identification_t *id;
		chunk_t data;

		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (payload->get_type(payload) == PLV2_ID_INITIATOR ||
				payload->get_type(payload) == PLV2_ID_RESPONDER)
			{
				id_payload = (id_payload_t*)payload;
				id = id_payload->get_identification(id_payload);
				data = id->get_encoding(id);

				DBG1(DBG_CFG, "%N: %N %B",
					 payload_type_short_names, payload->get_type(payload),
					 id_type_names, id->get_type(id), &data);
				id->destroy(id);
			}
		}
		enumerator->destroy(enumerator);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_log_id_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *log_id_hook_create(char *name)
{
	private_log_id_t *this;

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
