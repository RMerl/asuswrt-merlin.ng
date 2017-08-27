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

#include "stroke_handler.h"

#include <daemon.h>
#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_stroke_handler_t private_stroke_handler_t;

/**
 * Private data of an stroke_handler_t object.
 */
struct private_stroke_handler_t {

	/**
	 * Public stroke_handler_t interface.
	 */
	stroke_handler_t public;

	/**
	 * List of connection specific attributes, as attributes_t
	 */
	linked_list_t *attrs;

	/**
	 * rwlock to lock access to pools
	 */
	rwlock_t *lock;
};

/**
 * Attributes assigned to a connection
 */
typedef struct {
	/** name of the connection */
	char *name;
	/** list of DNS attributes, as host_t */
	linked_list_t *dns;
} attributes_t;

/**
 * Destroy an attributes_t entry
 */
static void attributes_destroy(attributes_t *this)
{
	this->dns->destroy_offset(this->dns, offsetof(host_t, destroy));
	free(this->name);
	free(this);
}

/**
 * Filter function to convert host to DNS configuration attributes
 */
static bool attr_filter(void *lock, host_t **in,
						configuration_attribute_type_t *type,
						void *dummy, chunk_t *data)
{
	host_t *host = *in;

	switch (host->get_family(host))
	{
		case AF_INET:
			*type = INTERNAL_IP4_DNS;
			break;
		case AF_INET6:
			*type = INTERNAL_IP6_DNS;
			break;
		default:
			return FALSE;
	}
	if (host->is_anyaddr(host))
	{
		*data = chunk_empty;
	}
	else
	{
		*data = host->get_address(host);
	}
	return TRUE;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t*,
	private_stroke_handler_t *this, identification_t *server,
	linked_list_t *vips)
{
	ike_sa_t *ike_sa;
	peer_cfg_t *peer_cfg;
	enumerator_t *enumerator;
	attributes_t *attr;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (ike_sa)
	{
		peer_cfg = ike_sa->get_peer_cfg(ike_sa);
		this->lock->read_lock(this->lock);
		enumerator = this->attrs->create_enumerator(this->attrs);
		while (enumerator->enumerate(enumerator, &attr))
		{
			if (streq(attr->name, peer_cfg->get_name(peer_cfg)))
			{
				enumerator->destroy(enumerator);
				return enumerator_create_filter(
									attr->dns->create_enumerator(attr->dns),
									(void*)attr_filter, this->lock,
									(void*)this->lock->unlock);
			}
		}
		enumerator->destroy(enumerator);
		this->lock->unlock(this->lock);
	}
	return enumerator_create_empty();
}

METHOD(stroke_handler_t, add_attributes, void,
	private_stroke_handler_t *this, stroke_msg_t *msg)
{
	if (msg->add_conn.me.dns)
	{
		enumerator_t *enumerator;
		attributes_t *attr = NULL;
		host_t *host;
		char *token;

		enumerator = enumerator_create_token(msg->add_conn.me.dns, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			if (streq(token, "%config") || streq(token, "%config4"))
			{
				host = host_create_any(AF_INET);
			}
			else if (streq(token, "%config6"))
			{
				host = host_create_any(AF_INET6);
			}
			else
			{
				host = host_create_from_string(token, 0);
			}
			if (host)
			{
				if (!attr)
				{
					INIT(attr,
						.name = strdup(msg->add_conn.name),
						.dns = linked_list_create(),
					);
				}
				attr->dns->insert_last(attr->dns, host);
			}
			else
			{
				DBG1(DBG_CFG, "ignoring invalid DNS address '%s'", token);
			}
		}
		enumerator->destroy(enumerator);
		if (attr)
		{
			this->lock->write_lock(this->lock);
			this->attrs->insert_last(this->attrs, attr);
			this->lock->unlock(this->lock);
		}
	}
}

METHOD(stroke_handler_t, del_attributes, void,
	private_stroke_handler_t *this, stroke_msg_t *msg)
{
	enumerator_t *enumerator;
	attributes_t *attr;

	this->lock->write_lock(this->lock);
	enumerator = this->attrs->create_enumerator(this->attrs);
	while (enumerator->enumerate(enumerator, &attr))
	{
		if (streq(msg->del_conn.name, attr->name))
		{
			this->attrs->remove_at(this->attrs, enumerator);
			attributes_destroy(attr);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(stroke_handler_t, destroy, void,
	private_stroke_handler_t *this)
{
	this->lock->destroy(this->lock);
	this->attrs->destroy_function(this->attrs, (void*)attributes_destroy);
	free(this);
}

/**
 * See header
 */
stroke_handler_t *stroke_handler_create()
{
	private_stroke_handler_t *this;

	INIT(this,
		.public = {
			.handler = {
				.handle = (void*)return_false,
				.release = (void*)return_false,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.add_attributes = _add_attributes,
			.del_attributes = _del_attributes,
			.destroy = _destroy,
		},
		.attrs = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
