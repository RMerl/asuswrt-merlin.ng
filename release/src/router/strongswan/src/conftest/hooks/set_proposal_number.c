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

typedef struct private_set_proposal_number_t private_set_proposal_number_t;

/**
 * Private data of an set_proposal_number_t object.
 */
struct private_set_proposal_number_t {

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
	 * Proposal number to modify
	 */
	int from;

	/**
	 * Proposal number to set
	 */
	int to;
};

/**
 * Copy all algs from given type from one proposal to another
 */
static void copy_proposal_algs(proposal_t *from, proposal_t *to,
							   transform_type_t type)
{
	enumerator_t *enumerator;
	uint16_t alg, key_size;

	enumerator = from->create_enumerator(from, type);
	while (enumerator->enumerate(enumerator, &alg, &key_size))
	{
		to->add_algorithm(to, type, alg, key_size);
	}
	enumerator->destroy(enumerator);
}

METHOD(listener_t, message, bool,
	private_set_proposal_number_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		enumerator_t *enumerator;
		payload_t *payload;
		linked_list_t *list = NULL, *updated;
		sa_payload_t *sa;
		proposal_t *proposal, *new;

		updated = linked_list_create();
		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (payload->get_type(payload) == PLV2_SECURITY_ASSOCIATION)
			{
				sa = (sa_payload_t*)payload;
				list = sa->get_proposals(sa);
				message->remove_payload_at(message, enumerator);
				sa->destroy(sa);
			}
		}
		enumerator->destroy(enumerator);

		if (list)
		{
			enumerator = list->create_enumerator(list);
			while (enumerator->enumerate(enumerator, &proposal))
			{
				if (proposal->get_number(proposal) == this->from)
				{
					DBG1(DBG_CFG, "setting proposal number from %d to %d",
						 this->from, this->to);
					new = proposal_create(proposal->get_protocol(proposal),
										  this->to);
					copy_proposal_algs(proposal, new, ENCRYPTION_ALGORITHM);
					copy_proposal_algs(proposal, new, INTEGRITY_ALGORITHM);
					copy_proposal_algs(proposal, new, PSEUDO_RANDOM_FUNCTION);
					copy_proposal_algs(proposal, new, DIFFIE_HELLMAN_GROUP);
					copy_proposal_algs(proposal, new, EXTENDED_SEQUENCE_NUMBERS);
					updated->insert_last(updated, new);
				}
				else
				{
					list->remove_at(list, enumerator);
					updated->insert_last(updated, proposal);
				}
			}
			enumerator->destroy(enumerator);
		}
		sa = sa_payload_create_from_proposals_v2(updated);
		DESTROY_OFFSET_IF(list, offsetof(proposal_t, destroy));
		updated->destroy_offset(updated, offsetof(proposal_t, destroy));
		message->add_payload(message, (payload_t*)sa);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_set_proposal_number_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *set_proposal_number_hook_create(char *name)
{
	private_set_proposal_number_t *this;

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
		.from = conftest->test->get_int(conftest->test,
										"hooks.%s.from", 0, name),
		.to = conftest->test->get_int(conftest->test,
										"hooks.%s.to", 1, name),
	);

	return &this->hook;
}
