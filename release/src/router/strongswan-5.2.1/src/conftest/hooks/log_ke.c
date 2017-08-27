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

#include <encoding/payloads/ke_payload.h>

typedef struct private_log_ke_t private_log_ke_t;

/**
 * Private data of an log_ke_t object.
 */
struct private_log_ke_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;
};

METHOD(listener_t, message, bool,
	private_log_ke_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (incoming && plain)
	{
		enumerator_t *enumerator;
		payload_t *payload;
		ke_payload_t *ke;

		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (payload->get_type(payload) == PLV2_KEY_EXCHANGE)
			{
				ke = (ke_payload_t*)payload;
				DBG1(DBG_CFG, "received DH group %N",
					 diffie_hellman_group_names, ke->get_dh_group_number(ke));
			}
		}
		enumerator->destroy(enumerator);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_log_ke_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *log_ke_hook_create(char *name)
{
	private_log_ke_t *this;

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
