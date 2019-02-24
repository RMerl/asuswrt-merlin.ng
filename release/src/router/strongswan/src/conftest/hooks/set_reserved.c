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

typedef struct private_set_reserved_t private_set_reserved_t;

/**
 * Private data of an set_reserved_t object.
 */
struct private_set_reserved_t {

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
	 * Hook name
	 */
	char *name;
};

/**
 * Set reserved bit of a payload
 */
static void set_bit(private_set_reserved_t *this, message_t *message,
					payload_type_t type, u_int nr)
{
	enumerator_t *payloads;
	payload_t *payload;
	bool *bit;

	if (type == PL_HEADER)
	{
		message->set_reserved_header_bit(message, nr);
		DBG1(DBG_CFG, "setting reserved bit %d of %N",
			  nr, payload_type_short_names, type);
	}
	else
	{
		payloads = message->create_payload_enumerator(message);
		while (payloads->enumerate(payloads, &payload))
		{
			if (payload->get_type(payload) == type)
			{
				bit = payload_get_field(payload, RESERVED_BIT, nr);
				if (bit)
				{
					DBG1(DBG_CFG, "setting reserved bit %d of %N",
						 nr, payload_type_short_names, type);
					*bit = TRUE;
				}
			}
		}
		payloads->destroy(payloads);
	}
}

/**
 * Set reserved byte of a payload
 */
static void set_byte(private_set_reserved_t *this, message_t *message,
					payload_type_t type, u_int nr, uint8_t byteval)
{
	enumerator_t *payloads;
	payload_t *payload;
	uint8_t *byte;

	if (type == PLV2_TRANSFORM_SUBSTRUCTURE || type == PLV2_PROPOSAL_SUBSTRUCTURE)
	{
		enumerator_t *transforms, *proposals;
		transform_substructure_t *transform;
		proposal_substructure_t *proposal;
		sa_payload_t *sa;

		payloads = message->create_payload_enumerator(message);
		while (payloads->enumerate(payloads, &payload))
		{
			if (payload->get_type(payload) == PLV2_SECURITY_ASSOCIATION)
			{
				sa = (sa_payload_t*)payload;
				proposals = sa->create_substructure_enumerator(sa);
				while (proposals->enumerate(proposals, &proposal))
				{
					if (type == PLV2_PROPOSAL_SUBSTRUCTURE)
					{
						byte = payload_get_field(&proposal->payload_interface,
												 RESERVED_BYTE, nr);
						if (byte)
						{
							DBG1(DBG_CFG, "setting reserved byte %d of %N to %d",
								 nr, payload_type_short_names, type, byteval);
							*byte = byteval;
						}
					}
					else if (type == PLV2_TRANSFORM_SUBSTRUCTURE)
					{
						transforms = proposal->create_substructure_enumerator(
																	proposal);
						while (transforms->enumerate(transforms, &transform))
						{
							byte = payload_get_field(&transform->payload_interface,
													 RESERVED_BYTE, nr);
							if (byte)
							{
								DBG1(DBG_CFG, "setting reserved byte %d of %N to %d",
									 nr, payload_type_short_names, type, byteval);
								*byte = byteval;
							}
						}
						transforms->destroy(transforms);
					}
				}
				proposals->destroy(proposals);
			}
		}
		payloads->destroy(payloads);
	}
	else
	{
		payloads = message->create_payload_enumerator(message);
		while (payloads->enumerate(payloads, &payload))
		{
			if (payload->get_type(payload) == type)
			{
				byte = payload_get_field(payload, RESERVED_BYTE, nr);
				if (byte)
				{
					DBG1(DBG_CFG, "setting reserved byte %d of %N to %d",
						  nr, payload_type_short_names, type, byteval);
					*byte = byteval;
				}
			}
		}
		payloads->destroy(payloads);
	}
}

METHOD(listener_t, message, bool,
	private_set_reserved_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		enumerator_t *bits, *bytes, *types;
		payload_type_t type;
		char *nr, *name;
		uint8_t byteval;

		types = conftest->test->create_section_enumerator(conftest->test,
													"hooks.%s", this->name);
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
			nr = conftest->test->get_str(conftest->test,
								"hooks.%s.%s.bits", "", this->name, name);
			bits = enumerator_create_token(nr, ",", " ");
			while (bits->enumerate(bits, &nr))
			{
				set_bit(this, message, type, atoi(nr));
			}
			bits->destroy(bits);

			nr = conftest->test->get_str(conftest->test,
								"hooks.%s.%s.bytes", "", this->name, name);
			byteval = conftest->test->get_int(conftest->test,
								"hooks.%s.%s.byteval", 255, this->name, name);
			bytes = enumerator_create_token(nr, ",", " ");
			while (bytes->enumerate(bytes, &nr))
			{
				set_byte(this, message, type, atoi(nr), byteval);
			}
			bytes->destroy(bytes);
		}
		types->destroy(types);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_set_reserved_t *this)
{
	free(this->name);
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *set_reserved_hook_create(char *name)
{
	private_set_reserved_t *this;

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
