/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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

#include "unity_provider.h"

#include <daemon.h>
#include <bio/bio_writer.h>

typedef struct private_unity_provider_t private_unity_provider_t;

/**
 * Private data of an unity_provider_t object.
 */
struct private_unity_provider_t {

	/**
	 * Public unity_provider_t interface.
	 */
	unity_provider_t public;
};

/**
 * Attribute enumerator for UNITY_SPLIT_INCLUDE attribute
 */
typedef struct {
	/** Implements enumerator_t */
	enumerator_t public;
	/** list of traffic selectors to enumerate */
	linked_list_t *list;
	/** attribute value */
	chunk_t attr;
} attribute_enumerator_t;

/**
 * Append data from the given traffic selector to the attribute data
 */
static void append_ts(bio_writer_t *writer, traffic_selector_t *ts)
{
	host_t *net, *mask;
	chunk_t padding;
	u_int8_t bits;

	if (!ts->to_subnet(ts, &net, &bits))
	{
		return;
	}
	mask = host_create_netmask(AF_INET, bits);
	if (!mask)
	{
		net->destroy(net);
		return;
	}
	writer->write_data(writer, net->get_address(net));
	writer->write_data(writer, mask->get_address(mask));
	/* the Cisco client parses the "padding" as protocol, src and dst port, the
	 * first two in network order the last in host order - no other clients seem
	 * to support these fields so we don't use them either */
	padding = writer->skip(writer, 6);
	memset(padding.ptr, 0, padding.len);
	mask->destroy(mask);
	net->destroy(net);
}

METHOD(enumerator_t, attribute_enumerate, bool,
	attribute_enumerator_t *this, configuration_attribute_type_t *type,
	chunk_t *attr)
{
	traffic_selector_t *ts;
	bio_writer_t *writer;

	if (this->list->get_count(this->list) == 0)
	{
		return FALSE;
	}

	writer = bio_writer_create(14);
	while (this->list->remove_first(this->list, (void**)&ts) == SUCCESS)
	{
		append_ts(writer, ts);
		ts->destroy(ts);
	}

	*type = UNITY_SPLIT_INCLUDE;
	*attr = this->attr = writer->extract_buf(writer);

	writer->destroy(writer);
	return TRUE;
}

METHOD(enumerator_t, attribute_destroy, void,
	attribute_enumerator_t *this)
{
	this->list->destroy_offset(this->list, offsetof(traffic_selector_t, destroy));
	chunk_free(&this->attr);
	free(this);
}

/**
 * Check if we should send a configured TS as Split-Include attribute
 */
static bool use_ts(traffic_selector_t *ts)
{
	u_int8_t mask;
	host_t *net;

	if (ts->get_type(ts) != TS_IPV4_ADDR_RANGE)
	{
		return FALSE;
	}
	if (ts->is_dynamic(ts))
	{
		return FALSE;
	}
	if (!ts->to_subnet(ts, &net, &mask))
	{
		return FALSE;
	}
	net->destroy(net);
	return mask > 0;
}

METHOD(attribute_provider_t, create_attribute_enumerator, enumerator_t*,
	private_unity_provider_t *this, linked_list_t *pools, identification_t *id,
	linked_list_t *vips)
{
	attribute_enumerator_t *attr_enum;
	enumerator_t *enumerator;
	linked_list_t *list, *current;
	traffic_selector_t *ts;
	ike_sa_t *ike_sa;
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa || ike_sa->get_version(ike_sa) != IKEV1 ||
		!ike_sa->supports_extension(ike_sa, EXT_CISCO_UNITY) ||
		!vips->get_count(vips))
	{
		return NULL;
	}

	list = linked_list_create();
	peer_cfg = ike_sa->get_peer_cfg(ike_sa);
	enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
	while (enumerator->enumerate(enumerator, &child_cfg))
	{
		current = child_cfg->get_traffic_selectors(child_cfg, TRUE, NULL, NULL);
		while (current->remove_first(current, (void**)&ts) == SUCCESS)
		{
			if (use_ts(ts))
			{
				list->insert_last(list, ts);
			}
			else
			{
				ts->destroy(ts);
			}
		}
		current->destroy(current);
	}
	enumerator->destroy(enumerator);

	if (list->get_count(list) == 0)
	{
		list->destroy(list);
		return NULL;
	}
	DBG1(DBG_CFG, "sending %N: %#R",
		 configuration_attribute_type_names, UNITY_SPLIT_INCLUDE, list);

	INIT(attr_enum,
		.public = {
			.enumerate = (void*)_attribute_enumerate,
			.destroy = _attribute_destroy,
		},
		.list = list,
	);

	return &attr_enum->public;
}

METHOD(unity_provider_t, destroy, void,
	private_unity_provider_t *this)
{
	free(this);
}

/**
 * See header
 */
unity_provider_t *unity_provider_create()
{
	private_unity_provider_t *this;

	INIT(this,
		.public = {
			.provider = {
				.acquire_address = (void*)return_null,
				.release_address = (void*)return_false,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.destroy = _destroy,
		},
	);

	return &this->public;
}
