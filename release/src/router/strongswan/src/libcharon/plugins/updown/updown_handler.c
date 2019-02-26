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

#include "updown_handler.h"

#include <daemon.h>
#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_updown_handler_t private_updown_handler_t;

/**
 * Private data of an updown_handler_t object.
 */
struct private_updown_handler_t {

	/**
	 * Public updown_handler_t interface.
	 */
	updown_handler_t public;

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
 * Attributes assigned to an IKE_SA
 */
typedef struct {
	/** unique IKE_SA identifier */
	u_int id;
	/** list of DNS attributes, as host_t */
	linked_list_t *dns;
} attributes_t;

/**
 * Destroy an attributes_t entry
 */
static void attributes_destroy(attributes_t *this)
{
	this->dns->destroy_offset(this->dns, offsetof(host_t, destroy));
	free(this);
}

METHOD(attribute_handler_t, handle, bool,
	private_updown_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	attributes_t *current, *attr = NULL;
	enumerator_t *enumerator;
	host_t *host;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			host = host_create_from_chunk(AF_INET, data, 0);
			break;
		case INTERNAL_IP6_DNS:
			host = host_create_from_chunk(AF_INET6, data, 0);
			break;
		default:
			return FALSE;
	}
	if (!host)
	{
		return FALSE;
	}

	this->lock->write_lock(this->lock);
	enumerator = this->attrs->create_enumerator(this->attrs);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current->id == ike_sa->get_unique_id(ike_sa))
		{
			attr = current;
		}
	}
	enumerator->destroy(enumerator);

	if (!attr)
	{
		INIT(attr,
			.id = ike_sa->get_unique_id(ike_sa),
			.dns = linked_list_create(),
		);
		this->attrs->insert_last(this->attrs, attr);
	}
	attr->dns->insert_last(attr->dns, host);
	this->lock->unlock(this->lock);

	return TRUE;
}

METHOD(attribute_handler_t, release, void,
	private_updown_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	attributes_t *attr;
	enumerator_t *enumerator, *servers;
	host_t *host;
	bool found = FALSE;
	int family;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			family = AF_INET;
			break;
		case INTERNAL_IP6_DNS:
			family = AF_INET6;
			break;
		default:
			return;
	}

	this->lock->write_lock(this->lock);
	enumerator = this->attrs->create_enumerator(this->attrs);
	while (enumerator->enumerate(enumerator, &attr))
	{
		if (attr->id == ike_sa->get_unique_id(ike_sa))
		{
			servers = attr->dns->create_enumerator(attr->dns);
			while (servers->enumerate(servers, &host))
			{
				if (host->get_family(host) == family &&
					chunk_equals(data, host->get_address(host)))
				{
					attr->dns->remove_at(attr->dns, servers);
					host->destroy(host);
					found = TRUE;
					break;
				}
			}
			servers->destroy(servers);
			if (attr->dns->get_count(attr->dns) == 0)
			{
				this->attrs->remove_at(this->attrs, enumerator);
				attributes_destroy(attr);
				break;
			}
		}
		if (found)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(updown_handler_t, create_dns_enumerator, enumerator_t*,
	private_updown_handler_t *this, u_int id)
{
	attributes_t *attr;
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		return enumerator_create_empty();
	}

	this->lock->read_lock(this->lock);
	enumerator = this->attrs->create_enumerator(this->attrs);
	while (enumerator->enumerate(enumerator, &attr))
	{
		if (attr->id == ike_sa->get_unique_id(ike_sa))
		{
			enumerator->destroy(enumerator);
			return enumerator_create_cleaner(
										attr->dns->create_enumerator(attr->dns),
										(void*)this->lock->unlock, this->lock);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return enumerator_create_empty();
}


METHOD(updown_handler_t, destroy, void,
	private_updown_handler_t *this)
{
	this->lock->destroy(this->lock);
	this->attrs->destroy_function(this->attrs, (void*)attributes_destroy);
	free(this);
}

/**
 * See header
 */
updown_handler_t *updown_handler_create()
{
	private_updown_handler_t *this;

	INIT(this,
		.public = {
			.handler = {
				.handle = _handle,
				.release = _release,
				.create_attribute_enumerator = enumerator_create_empty,
			},
			.create_dns_enumerator = _create_dns_enumerator,
			.destroy = _destroy,
		},
		.attrs = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
