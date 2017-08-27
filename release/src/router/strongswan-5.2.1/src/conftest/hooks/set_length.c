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

typedef struct private_set_length_t private_set_length_t;

/**
 * Private data of an set_length_t object.
 */
struct private_set_length_t {

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
	 * Difference to correct length
	 */
	int diff;
};

METHOD(listener_t, message, bool,
	private_set_length_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		payload_t *payload;
		enumerator_t *enumerator;
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
		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (type == payload->get_type(payload))
			{
				encoding_rule_t *rules;
				u_int16_t *len;
				int i, count;

				count = payload->get_encoding_rules(payload, &rules);
				for (i = 0; i < count; i++)
				{
					if (rules[i].type == PAYLOAD_LENGTH)
					{
						len = (u_int16_t*)(((void*)payload) + rules[i].offset);
						DBG1(DBG_CFG, "adjusting length of %N payload "
							 "from %d to %d", payload_type_short_names, type,
							 *len, *len + this->diff);
						*len = *len + this->diff;
					}
				}
			}
		}
		enumerator->destroy(enumerator);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_set_length_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *set_length_hook_create(char *name)
{
	private_set_length_t *this;

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
		.diff = conftest->test->get_int(conftest->test,
										"hooks.%s.diff", 0, name),
	);

	return &this->hook;
}
