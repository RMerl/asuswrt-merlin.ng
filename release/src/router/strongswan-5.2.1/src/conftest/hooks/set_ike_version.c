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

typedef struct private_set_ike_version_t private_set_ike_version_t;

/**
 * Private data of an set_ike_version_t object.
 */
struct private_set_ike_version_t {

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
	 * Major version to set
	 */
	int major;

	/**
	 * Minor version to set
	 */
	int minor;

	/**
	 * Higher version supported?
	 */
	bool higher;
};

METHOD(listener_t, message, bool,
	private_set_ike_version_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		DBG1(DBG_CFG, "setting IKE version of message ID %d to %d.%d",
			 this->id, this->major, this->minor);
		message->set_major_version(message, this->major);
		message->set_minor_version(message, this->minor);
		if (this->higher)
		{
			message->set_version_flag(message);
		}
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_set_ike_version_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *set_ike_version_hook_create(char *name)
{
	private_set_ike_version_t *this;

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
		.major = conftest->test->get_int(conftest->test,
										"hooks.%s.major", 2, name),
		.minor = conftest->test->get_int(conftest->test,
										"hooks.%s.minor", 0, name),
		.higher = conftest->test->get_bool(conftest->test,
										"hooks.%s.higher", FALSE, name),
	);

	return &this->hook;
}
