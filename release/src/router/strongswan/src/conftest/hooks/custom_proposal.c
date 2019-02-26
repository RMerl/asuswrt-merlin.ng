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

#include <errno.h>

#include <encoding/payloads/sa_payload.h>
#include <crypto/proposal/proposal.h>

typedef struct private_custom_proposal_t private_custom_proposal_t;

/**
 * Private data of an custom_proposal_t object.
 */
struct private_custom_proposal_t {

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
	 * hook name
	 */
	char *name;
};

/**
 * Load custom proposal configuration to proposal list
 */
static linked_list_t* load_proposals(private_custom_proposal_t *this,
									 protocol_id_t proto, uint64_t spi)
{
	enumerator_t *props, *algs;
	char *number, *key, *value;
	linked_list_t *list;

	list = linked_list_create();
	props = conftest->test->create_section_enumerator(conftest->test,
													  "hooks.%s", this->name);
	while (props->enumerate(props, &number))
	{
		const proposal_token_t *token = NULL;
		proposal_t *proposal;
		uint16_t type, alg, keysize = 0;
		char *end;

		proposal = proposal_create(proto, atoi(number));
		proposal->set_spi(proposal, spi);

		algs = conftest->test->create_key_value_enumerator(conftest->test,
											"hooks.%s.%s", this->name, number);
		while (algs->enumerate(algs, &key, &value))
		{
			errno = 0;
			type = strtoul(key, &end, 10);
			if (end == key || errno)
			{
				if (!enum_from_name(transform_type_names, key, &type))
				{
					DBG1(DBG_CFG, "unknown transform: '%s', skipped", key);
					continue;
				}
			}
			errno = 0;
			alg = strtoul(value, &end, 10);
			if (end == value || errno)
			{
				token = lib->proposal->get_token(lib->proposal, value);
				if (!token)
				{
					DBG1(DBG_CFG, "unknown algorithm: '%s', skipped", value);
					continue;
				}
				keysize = token->keysize;
				alg = token->algorithm;
			}
			proposal->add_algorithm(proposal, type, alg, keysize);
		}
		algs->destroy(algs);
		list->insert_last(list, proposal);
	}
	props->destroy(props);
	return list;
}

METHOD(listener_t, message, bool,
	private_custom_proposal_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		enumerator_t *enumerator;
		payload_t *payload;
		sa_payload_t *new, *old = NULL;
		linked_list_t *new_props, *old_props;
		proposal_t *proposal;

		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (payload->get_type(payload) == PLV2_SECURITY_ASSOCIATION)
			{
				old = (sa_payload_t*)payload;
				message->remove_payload_at(message, enumerator);
			}
		}
		enumerator->destroy(enumerator);

		if (old)
		{
			old_props = old->get_proposals(old);
			old->destroy(old);
			enumerator = old_props->create_enumerator(old_props);
			if (enumerator->enumerate(enumerator, &proposal))
			{
				new_props = load_proposals(this,
										   proposal->get_protocol(proposal),
										   proposal->get_spi(proposal));
				DBG1(DBG_CFG, "injecting custom proposal: %#P", new_props);
				new = sa_payload_create_from_proposals_v2(new_props);
				message->add_payload(message, (payload_t*)new);
				new_props->destroy_offset(new_props, offsetof(proposal_t, destroy));
			}
			enumerator->destroy(enumerator);
			old_props->destroy_offset(old_props, offsetof(proposal_t, destroy));
		}
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_custom_proposal_t *this)
{
	free(this->name);
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *custom_proposal_hook_create(char *name)
{
	private_custom_proposal_t *this;

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
		.name = strdup(name),
	);

	return &this->hook;
}
