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

#include <encoding/payloads/ts_payload.h>

typedef struct private_log_ts_t private_log_ts_t;

/**
 * Private data of an log_ts_t object.
 */
struct private_log_ts_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;
};

METHOD(listener_t, message, bool,
	private_log_ts_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (incoming && plain)
	{
		enumerator_t *enumerator;
		payload_t *payload;
		ts_payload_t *ts;

		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (payload->get_type(payload) == PLV2_TS_INITIATOR ||
				payload->get_type(payload) == PLV2_TS_RESPONDER)
			{
				ts = (ts_payload_t*)payload;
				host_t *from, *to;
				linked_list_t *list;
				enumerator_t *tsenum;
				traffic_selector_t *selector;

				list = ts->get_traffic_selectors(ts);
				tsenum = list->create_enumerator(list);
				while (tsenum->enumerate(tsenum, &selector))
				{
					from = host_create_from_chunk(AF_UNSPEC,
									selector->get_from_address(selector), 0);
					to = host_create_from_chunk(AF_UNSPEC,
									selector->get_to_address(selector), 0);

					DBG1(DBG_CFG, "received %N: %N %H-%H proto %u port %u-%u",
						 payload_type_short_names, payload->get_type(payload),
						 ts_type_name, selector->get_type(selector),
						 from, to, selector->get_protocol(selector),
						 selector->get_from_port(selector),
						 selector->get_to_port(selector));
				}
				tsenum->destroy(tsenum);

				list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
			}
		}
		enumerator->destroy(enumerator);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_log_ts_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *log_ts_hook_create(char *name)
{
	private_log_ts_t *this;

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
