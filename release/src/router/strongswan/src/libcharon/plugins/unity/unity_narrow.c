/*
 * Copyright (C) 2014 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "unity_narrow.h"

#include <daemon.h>
#include <encoding/payloads/id_payload.h>
#include <collections/hashtable.h>

typedef struct private_unity_narrow_t private_unity_narrow_t;

/**
 * Private data of an unity_narrow_t object.
 */
struct private_unity_narrow_t {

	/**
	 * Public unity_narrow_t interface.
	 */
	unity_narrow_t public;

	/**
	 * Unity attribute handler
	 */
	unity_handler_t *handler;

	/**
	 * IKE_SAs for which we received 0.0.0.0/0 as remote traffic selector
	 */
	hashtable_t *wildcard_ts;
};

/**
 * Narrow the given received traffic selector with the child configuration and
 * put them into the given list of TS
 */
static void narrow_ts(child_cfg_t *cfg, traffic_selector_t *ts,
					  linked_list_t *list)
{
	linked_list_t *received, *selected;

	received = linked_list_create();
	received->insert_last(received, ts);
	selected = cfg->get_traffic_selectors(cfg, FALSE, received, NULL, FALSE);
	while (selected->remove_first(selected, (void**)&ts) == SUCCESS)
	{
		list->insert_last(list, ts);
	}
	selected->destroy(selected);
	received->destroy(received);
}

/**
 * Narrow TS as initiator to Unity Split-Include/Local-LAN
 */
static void narrow_initiator(private_unity_narrow_t *this, ike_sa_t *ike_sa,
							 child_cfg_t *cfg, linked_list_t *remote)
{
	traffic_selector_t *current, *orig = NULL;
	enumerator_t *enumerator;

	enumerator = this->handler->create_include_enumerator(this->handler,
											ike_sa->get_id(ike_sa));
	while (enumerator->enumerate(enumerator, &current))
	{
		if (orig == NULL)
		{	/* got one, replace original TS */
			if (remote->remove_first(remote, (void**)&orig) != SUCCESS)
			{
				break;
			}
		}
		narrow_ts(cfg, current, remote);
	}
	enumerator->destroy(enumerator);
	if (orig)
	{
		DBG1(DBG_CFG, "narrowed CHILD_SA to %N %#R",
			 configuration_attribute_type_names,
			 UNITY_SPLIT_INCLUDE, remote);
		orig->destroy(orig);
	}
	else
	{	/* since we originally changed the traffic selector to 0.0.0.0/0 local
		 * narrowing is not applied if no Split-Include attrs are received */
		if (remote->remove_first(remote, (void**)&orig) == SUCCESS)
		{
			narrow_ts(cfg, orig, remote);
			orig->destroy(orig);
		}
	}
}

/**
 * As initiator and responder, bump up TS to 0.0.0.0/0 for on-the-wire bits
 */
static void narrow_pre(linked_list_t *list, char *side)
{
	traffic_selector_t *ts;

	while (list->remove_first(list, (void**)&ts) == SUCCESS)
	{
		ts->destroy(ts);
	}
	ts = traffic_selector_create_from_string(0, TS_IPV4_ADDR_RANGE,
											 "0.0.0.0", 0,
											 "255.255.255.255", 65535);
	if (ts)
	{
		DBG2(DBG_CFG, "changing proposed traffic selectors for %s:", side);
		DBG2(DBG_CFG, " %R", ts);
		list->insert_last(list, ts);
	}
}

/**
 * As responder, narrow down TS to configuration for installation
 */
static void narrow_responder_post(child_cfg_t *child_cfg, linked_list_t *local)
{
	traffic_selector_t *ts;
	linked_list_t *configured;

	while (local->remove_first(local, (void**)&ts) == SUCCESS)
	{
		ts->destroy(ts);
	}
	configured = child_cfg->get_traffic_selectors(child_cfg, TRUE, NULL, NULL,
												  FALSE);

	while (configured->remove_first(configured, (void**)&ts) == SUCCESS)
	{
		local->insert_last(local, ts);
	}
	configured->destroy(configured);
}

/**
 * Check if any Split-Include attributes are active on this IKE_SA
 */
static bool has_split_includes(private_unity_narrow_t *this, ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	bool has;

	enumerator = this->handler->create_include_enumerator(this->handler,
												ike_sa->get_id(ike_sa));
	has = enumerator->enumerate(enumerator, &ts);
	enumerator->destroy(enumerator);

	return has;
}

METHOD(listener_t, narrow, bool,
	private_unity_narrow_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	narrow_hook_t type, linked_list_t *local, linked_list_t *remote)
{
	if (ike_sa->get_version(ike_sa) == IKEV1 &&
		ike_sa->supports_extension(ike_sa, EXT_CISCO_UNITY))
	{
		/* depending on who initiates a rekeying the hooks will not match the
		 * roles in the IKE_SA */
		if (ike_sa->has_condition(ike_sa, COND_ORIGINAL_INITIATOR))
		{
			switch (type)
			{
				case NARROW_INITIATOR_PRE_AUTH:
				case NARROW_RESPONDER:
					if (has_split_includes(this, ike_sa))
					{
						narrow_pre(remote, "other");
					}
					break;
				case NARROW_INITIATOR_POST_AUTH:
				case NARROW_RESPONDER_POST:
					narrow_initiator(this, ike_sa,
									 child_sa->get_config(child_sa), remote);
					break;
				default:
					break;
			}
		}
		else
		{
			switch (type)
			{
				case NARROW_INITIATOR_PRE_AUTH:
				case NARROW_RESPONDER:
					if (this->wildcard_ts->get(this->wildcard_ts, ike_sa))
					{
						narrow_pre(local, "us");

					}
					break;
				case NARROW_INITIATOR_POST_AUTH:
				case NARROW_RESPONDER_POST:
					if (this->wildcard_ts->get(this->wildcard_ts, ike_sa))
					{
						narrow_responder_post(child_sa->get_config(child_sa),
											  local);
					}
					break;
				default:
					break;
			}
		}
	}
	return TRUE;
}

METHOD(listener_t, message, bool,
	private_unity_narrow_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	traffic_selector_t *tsr = NULL, *wildcard;
	enumerator_t *enumerator;
	id_payload_t *id_payload;
	payload_t *payload;
	bool first = TRUE;

	if (!incoming || !plain ||
		message->get_exchange_type(message) != QUICK_MODE ||
		!ike_sa || !ike_sa->supports_extension(ike_sa, EXT_CISCO_UNITY))
	{
		return TRUE;
	}
	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV1_ID)
		{
			if (!first)
			{
				id_payload = (id_payload_t*)payload;
				tsr = id_payload->get_ts(id_payload);
				break;
			}
			first = FALSE;
		}
	}
	enumerator->destroy(enumerator);
	if (!tsr)
	{
		return TRUE;
	}
	wildcard = traffic_selector_create_from_cidr("0.0.0.0/0", 0, 0, 65535);
	if (tsr->equals(tsr, wildcard))
	{
		this->wildcard_ts->put(this->wildcard_ts, ike_sa, ike_sa);
	}
	else
	{
		this->wildcard_ts->remove(this->wildcard_ts, ike_sa);
	}
	wildcard->destroy(wildcard);
	tsr->destroy(tsr);
	return TRUE;
}

METHOD(listener_t, ike_updown, bool,
	private_unity_narrow_t *this, ike_sa_t *ike_sa, bool up)
{
	if (!up)
	{
		this->wildcard_ts->remove(this->wildcard_ts, ike_sa);
	}
	return TRUE;
}

METHOD(unity_narrow_t, destroy, void,
	private_unity_narrow_t *this)
{
	this->wildcard_ts->destroy(this->wildcard_ts);
	free(this);
}

/**
 * See header
 */
unity_narrow_t *unity_narrow_create(unity_handler_t *handler)
{
	private_unity_narrow_t *this;

	INIT(this,
		.public = {
			.listener = {
				.narrow = _narrow,
				.message = _message,
				.ike_updown = _ike_updown,
			},
			.destroy = _destroy,
		},
		.handler = handler,
		.wildcard_ts = hashtable_create(hashtable_hash_ptr,
										hashtable_equals_ptr, 4),
	);

	return &this->public;
}
