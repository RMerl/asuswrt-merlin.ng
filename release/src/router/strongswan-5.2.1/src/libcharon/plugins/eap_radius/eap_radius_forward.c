/*
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

#include "eap_radius_forward.h"

#include <daemon.h>
#include <collections/linked_list.h>
#include <collections/hashtable.h>
#include <threading/mutex.h>

typedef struct private_eap_radius_forward_t private_eap_radius_forward_t;

/**
 * Private data of an eap_radius_forward_t object.
 */
struct private_eap_radius_forward_t {

	/**
	 * Public eap_radius_forward_t interface.
	 */
	eap_radius_forward_t public;

	/**
	 * List of attribute types to copy from IKE, as attr_t
	 */
	linked_list_t *from_attr;

	/**
	 * List of attribute types to copy to IKE, as attr_t
	 */
	linked_list_t *to_attr;

	/**
	 * Queued to forward from IKE, unique_id => linked_list_t of chunk_t
	 */
	hashtable_t *from;

	/**
	 * Queued to forward to IKE, unique_id => linked_list_t of chunk_t
	 */
	hashtable_t *to;

	/**
	 * Mutex to lock concurrent access to hashtables
	 */
	mutex_t *mutex;
};

/**
 * RADIUS attribute selector
 */
typedef struct {
	/** vendor ID, 0 for standard attributes */
	u_int32_t vendor;
	/** attribute type */
	u_int8_t type;
} attr_t;

/**
 * Single instance of this
 */
static private_eap_radius_forward_t *singleton = NULL;

/**
 * Free a queue entry
 */
static void free_attribute(chunk_t *chunk)
{
	free(chunk->ptr);
	free(chunk);
}

/**
 * Lookup/create an attribute queue from a table
 */
static linked_list_t *lookup_queue(private_eap_radius_forward_t *this,
								   hashtable_t *table)
{
	linked_list_t *queue = NULL;
	ike_sa_t *ike_sa;
	uintptr_t id;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (ike_sa && ike_sa->supports_extension(ike_sa, EXT_STRONGSWAN))
	{
		id = ike_sa->get_unique_id(ike_sa);
		this->mutex->lock(this->mutex);
		queue = table->get(table, (void*)id);
		if (!queue)
		{
			queue = linked_list_create();
			table->put(table, (void*)id, queue);
		}
		this->mutex->unlock(this->mutex);
	}
	return queue;
}

/**
 * Remove attribute queue from table
 */
static void remove_queue(private_eap_radius_forward_t *this,
						 hashtable_t *table, ike_sa_t *ike_sa)
{
	linked_list_t *queue;

	this->mutex->lock(this->mutex);
	queue = table->remove(table, (void*)(uintptr_t)ike_sa->get_unique_id(ike_sa));
	this->mutex->unlock(this->mutex);
	if (queue)
	{
		queue->destroy_function(queue, (void*)free_attribute);
	}
}

/**
 * Check if RADIUS attribute is contained in selector
 */
static bool is_attribute_selected(linked_list_t *selector,
								  radius_attribute_type_t type, chunk_t data)
{
	enumerator_t *enumerator;
	u_int32_t vendor = 0;
	attr_t *sel;
	bool found = FALSE;

	if (type == RAT_VENDOR_SPECIFIC)
	{
		if (data.len < 4)
		{
			return FALSE;
		}
		vendor = untoh32(data.ptr);
	}
	enumerator = selector->create_enumerator(selector);
	while (!found && enumerator->enumerate(enumerator, &sel))
	{
		if (sel->vendor == vendor)
		{
			if (vendor)
			{
				if (sel->type == 0)
				{	/* any of that vendor is fine */
					found = TRUE;
				}
				else if (data.len > 4 && data.ptr[4] == sel->type)
				{	/* vendor specific type field, as defined in RFC 2865 */
					found = TRUE;
				}
			}
			else
			{
				if (sel->type == type)
				{
					found = TRUE;
				}
			}
		}
	}
	enumerator->destroy(enumerator);

	return found;
}

/**
 * Copy RADIUS attributes from queue to a RADIUS message
 */
static void queue2radius(linked_list_t *queue, radius_message_t *message)
{
	chunk_t *data;

	while (queue->remove_last(queue, (void**)&data) == SUCCESS)
	{
		if (data->len >= 2)
		{
			message->add(message, data->ptr[0], chunk_skip(*data, 2));
		}
		free_attribute(data);
	}
}

/**
 * Copy RADIUS attributes from a RADIUS message to the queue
 */
static void radius2queue(radius_message_t *message, linked_list_t *queue,
						 linked_list_t *selector)
{
	enumerator_t *enumerator;
	int type;
	chunk_t data, hdr, *ptr;

	enumerator = message->create_enumerator(message);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		if (is_attribute_selected(selector, type, data))
		{
			hdr = chunk_alloc(2);
			hdr.ptr[0] = type;
			hdr.ptr[1] = data.len + 2;

			INIT(ptr);
			*ptr = chunk_cat("mc", hdr, data);
			queue->insert_last(queue, ptr);
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Copy RADIUS attribute nofifies from IKE message to queue
 */
static void ike2queue(message_t *message, linked_list_t *queue,
					  linked_list_t *selector)
{
	enumerator_t *enumerator;
	payload_t *payload;
	notify_payload_t *notify;
	chunk_t data, *ptr;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV2_NOTIFY ||
			payload->get_type(payload) == PLV1_NOTIFY)
		{
			notify = (notify_payload_t*)payload;
			if (notify->get_notify_type(notify) == RADIUS_ATTRIBUTE)
			{
				data = notify->get_notification_data(notify);
				if (data.len >= 2 && is_attribute_selected(selector,
										data.ptr[0], chunk_skip(data, 2)))
				{
					INIT(ptr);
					*ptr = chunk_clone(data);
					queue->insert_last(queue, ptr);
				}
			}
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Copy RADUIS attributes from queue to IKE message notifies
 */
static void queue2ike(linked_list_t *queue, message_t *message)
{
	chunk_t *data;

	while (queue->remove_last(queue, (void**)&data) == SUCCESS)
	{
		message->add_notify(message, FALSE, RADIUS_ATTRIBUTE, *data);
		free_attribute(data);
	}
}

/**
 * See header.
 */
void eap_radius_forward_from_ike(radius_message_t *request)
{
	private_eap_radius_forward_t *this = singleton;
	linked_list_t *queue;

	if (this)
	{
		queue = lookup_queue(this, this->from);
		if (queue)
		{
			queue2radius(queue, request);
		}
	}
}

/**
 * See header.
 */
void eap_radius_forward_to_ike(radius_message_t *response)
{
	private_eap_radius_forward_t *this = singleton;
	linked_list_t *queue;

	if (this)
	{
		queue = lookup_queue(this, this->to);
		if (queue)
		{
			radius2queue(response, queue, this->to_attr);
		}
	}
}

METHOD(listener_t, message, bool,
	private_eap_radius_forward_t *this,
	ike_sa_t *ike_sa, message_t *message, bool incoming, bool plain)
{
	linked_list_t *queue;

	if (plain && message->get_exchange_type(message) == IKE_AUTH)
	{
		if (incoming)
		{
			queue = lookup_queue(this, this->from);
			if (queue)
			{
				ike2queue(message, queue, this->from_attr);
			}
		}
		else
		{
			queue = lookup_queue(this, this->to);
			if (queue)
			{
				queue2ike(queue, message);
			}
		}
	}
	return TRUE;
}

METHOD(listener_t, ike_updown, bool,
	private_eap_radius_forward_t *this, ike_sa_t *ike_sa, bool up)
{
	/* up or down, we don't need the state anymore */
	remove_queue(this, this->from, ike_sa);
	remove_queue(this, this->to, ike_sa);
	return TRUE;
}

/**
 * Parse a selector string to a list of attr_t selectors
 */
static linked_list_t* parse_selector(char *selector)
{
	enumerator_t *enumerator;
	linked_list_t *list;
	char *token, *pos;

	list = linked_list_create();
	enumerator = enumerator_create_token(selector, ",", " ");
	while (enumerator->enumerate(enumerator, &token))
	{
		int type, vendor = 0;
		attr_t *attr;

		pos = strchr(token, ':');
		if (pos)
		{
			*(pos++) = 0;
			vendor = atoi(token);
			token = pos;
		}
		if (!enum_from_name(radius_attribute_type_names, token, &type))
		{
			type = atoi(token);
		}
		if (vendor == 0 && type == 0)
		{
			DBG1(DBG_CFG, "ignoring unknown RADIUS attribute type '%s'", token);
		}
		else
		{
			INIT(attr,
				.type = type,
				.vendor = vendor,
			);
			list->insert_last(list, attr);
			if (!vendor)
			{
				DBG1(DBG_IKE, "forwarding RADIUS attribute %N",
					 radius_attribute_type_names, type);
			}
			else
			{
				DBG1(DBG_IKE, "forwarding RADIUS VSA %d-%d", vendor, type);
			}
		}
	}
	enumerator->destroy(enumerator);
	return list;
}

METHOD(eap_radius_forward_t, destroy, void,
	private_eap_radius_forward_t *this)
{
	this->from_attr->destroy_function(this->from_attr, free);
	this->to_attr->destroy_function(this->to_attr, free);
	this->from->destroy(this->from);
	this->to->destroy(this->to);
	this->mutex->destroy(this->mutex);
	free(this);
	singleton = NULL;
}

/**
 * See header
 */
eap_radius_forward_t *eap_radius_forward_create()
{
	private_eap_radius_forward_t *this;

	INIT(this,
		.public = {
			.listener = {
				.message = _message,
				.ike_updown = _ike_updown,
			},
			.destroy = _destroy,
		},
		.from_attr = parse_selector(lib->settings->get_str(lib->settings,
							"%s.plugins.eap-radius.forward.ike_to_radius", "",
							lib->ns)),
		.to_attr = parse_selector(lib->settings->get_str(lib->settings,
							"%s.plugins.eap-radius.forward.radius_to_ike", "",
							lib->ns)),
		.from = hashtable_create(hashtable_hash_ptr, hashtable_equals_ptr, 8),
		.to = hashtable_create(hashtable_hash_ptr, hashtable_equals_ptr, 8),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	if (this->from_attr->get_count(this->from_attr) == 0 &&
		this->to_attr->get_count(this->to_attr) == 0)
	{
		destroy(this);
		return NULL;
	}

	singleton = this;
	return &this->public;
}
