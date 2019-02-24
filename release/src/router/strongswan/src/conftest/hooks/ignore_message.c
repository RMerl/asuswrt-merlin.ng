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

typedef struct private_ignore_message_t private_ignore_message_t;

/**
 * Private data of an ignore_message_t object.
 */
struct private_ignore_message_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;

	/**
	 * Drop incoming or outgoing?
	 */
	bool in;

	/**
	 * Drop requests or responses?
	 */
	bool req;

	/**
	 * ID of message to drop.
	 */
	int id;
};

METHOD(listener_t, message, bool,
	private_ignore_message_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (incoming == this->in && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		DBG1(DBG_CFG, "ignoring message");
		message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_ignore_message_t *this)
{
	free(this);
}

/**
 * Create the ignore_message hook
 */
hook_t *ignore_message_hook_create(char *name)
{
	private_ignore_message_t *this;

	INIT(this,
		.hook = {
			.listener = {
				.message = _message,
			},
			.destroy = _destroy,
		},
		.in = conftest->test->get_bool(conftest->test,
										"hooks.%s.inbound", TRUE, name),
		.req = conftest->test->get_bool(conftest->test,
										"hooks.%s.request", TRUE, name),
		.id = conftest->test->get_int(conftest->test,
										"hooks.%s.id", 0, name),
	);

	return &this->hook;
}
