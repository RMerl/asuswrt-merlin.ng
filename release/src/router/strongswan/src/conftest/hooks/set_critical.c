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

typedef struct private_set_critical_t private_set_critical_t;

/**
 * Private data of an set_critical_t object.
 */
struct private_set_critical_t {

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
	 * Payload types, space separated
	 */
	char *payloads;
};

METHOD(listener_t, message, bool,
	private_set_critical_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		enumerator_t *msg, *types;
		payload_t *payload;
		payload_type_t type;
		bool *critical;
		char *name;

		types = enumerator_create_token(this->payloads, " ", "");
		while (types->enumerate(types, &name))
		{
			type = atoi(name);
			if (!type)
			{
				if (!enum_from_name(payload_type_short_names, name, &type))
				{
					DBG1(DBG_CFG, "invalid payload name '%s'", name);
					break;
				}
			}
			msg = message->create_payload_enumerator(message);
			while (msg->enumerate(msg, &payload))
			{
				if (type == payload->get_type(payload))
				{
					critical = payload_get_field(payload, FLAG, 0);
					if (critical)
					{
						*critical = TRUE;
					}
				}
			}
			msg->destroy(msg);
		}
		types->destroy(types);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_set_critical_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *set_critical_hook_create(char *name)
{
	private_set_critical_t *this;

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
		.payloads = conftest->test->get_str(conftest->test,
										"hooks.%s.payloads", "", name),
	);

	return &this->hook;
}
