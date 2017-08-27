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

#include <encoding/payloads/sa_payload.h>

typedef struct private_log_proposals_t private_log_proposals_t;

/**
 * Private data of an log_proposals_t object.
 */
struct private_log_proposals_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;
};

METHOD(listener_t, message, bool,
	private_log_proposals_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (incoming && plain)
	{
		enumerator_t *enumerator, *proposals;
		payload_t *payload;
		linked_list_t *list;
		sa_payload_t *sa;
		proposal_t *proposal;

		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (payload->get_type(payload) == PLV2_SECURITY_ASSOCIATION)
			{
				sa = (sa_payload_t*)payload;
				list = sa->get_proposals(sa);
				DBG1(DBG_CFG, "received %d proposal%s:", list->get_count(list),
					 list->get_count(list) == 1 ? "" : "s");
				proposals = list->create_enumerator(list);
				while (proposals->enumerate(proposals, &proposal))
				{
					u_int64_t spi = proposal->get_spi(proposal);

					if (proposal->get_protocol(proposal) != PROTO_IKE)
					{
						spi = htonl(spi);
					}
					DBG1(DBG_CFG, "  %d (SPI 0x%llx): %P",
						 proposal->get_number(proposal), spi, proposal);
				}
				proposals->destroy(proposals);
				list->destroy_offset(list, offsetof(proposal_t, destroy));
			}
		}
		enumerator->destroy(enumerator);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_log_proposals_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *log_proposals_hook_create(char *name)
{
	private_log_proposals_t *this;

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
