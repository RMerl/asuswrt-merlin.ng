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

typedef struct private_add_payload_t private_add_payload_t;

/**
 * Private data of an add_payload_t object.
 */
struct private_add_payload_t {

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
	 * Payload type
	 */
	char *type;

	/**
	 * Payload data
	 */
	char *data;

	/**
	 * Set critical bit of the payload
	 */
	bool critical;

	/**
	 * True to replace existing payload of this type
	 */
	bool replace;
};

METHOD(listener_t, message, bool,
	private_add_payload_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		unknown_payload_t *unknown;
		payload_t *payload;
		enumerator_t *enumerator;
		chunk_t data = chunk_empty;
		payload_type_t type;

		type = atoi(this->type);
		if (!type)
		{
			if (!enum_from_name(payload_type_short_names, this->type, &type))
			{
				DBG1(DBG_CFG, "unknown payload: '%s', skipped", this->type);
				return TRUE;
			}
		}
		if (this->replace)
		{
			enumerator = message->create_payload_enumerator(message);
			while (enumerator->enumerate(enumerator, &payload))
			{
				if (payload->get_type(payload) == type)
				{
					message->remove_payload_at(message, enumerator);
					payload->destroy(payload);
					break;
				}
			}
			enumerator->destroy(enumerator);
		}
		if (strncaseeq(this->data, "0x", 2))
		{
			data = chunk_skip(chunk_create(this->data, strlen(this->data)), 2);
			data = chunk_from_hex(data, NULL);
		}
		else if (strlen(this->data))
		{
			data = chunk_clone(chunk_create(this->data, strlen(this->data)));
		}
		unknown = unknown_payload_create_data(type, this->critical, data);
		message->add_payload(message, &unknown->payload_interface);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_add_payload_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *add_payload_hook_create(char *name)
{
	private_add_payload_t *this;

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
										"hooks.%s.id", 0, name),
		.type = conftest->test->get_str(conftest->test,
										"hooks.%s.type", "", name),
		.data = conftest->test->get_str(conftest->test,
										"hooks.%s.data", "", name),
		.critical = conftest->test->get_bool(conftest->test,
										"hooks.%s.critical", FALSE, name),
		.replace = conftest->test->get_bool(conftest->test,
										"hooks.%s.replace", FALSE, name),
	);

	return &this->hook;
}
