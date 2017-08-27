/*
 * Copyright (C) 2010 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include "stroke_attribute.h"

#include <daemon.h>
#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_stroke_attribute_t private_stroke_attribute_t;

/**
 * private data of stroke_attribute
 */
struct private_stroke_attribute_t {

	/**
	 * public functions
	 */
	stroke_attribute_t public;

	/**
	 * list of pools, contains mem_pool_t
	 */
	linked_list_t *pools;

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
 * find a pool by name
 */
static mem_pool_t *find_pool(private_stroke_attribute_t *this, char *name)
{
	enumerator_t *enumerator;
	mem_pool_t *current, *found = NULL;

	enumerator = this->pools->create_enumerator(this->pools);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (streq(name, current->get_name(current)))
		{
			found = current;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * Find an existing or not yet existing lease
 */
static host_t *find_addr(private_stroke_attribute_t *this, linked_list_t *pools,
						 identification_t *id, host_t *requested,
						 mem_pool_op_t operation)
{
	host_t *addr = NULL;
	enumerator_t *enumerator;
	mem_pool_t *pool;
	char *name;

	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &name))
	{
		pool = find_pool(this, name);
		if (pool)
		{
			addr = pool->acquire_address(pool, id, requested, operation);
			if (addr)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);

	return addr;
}

METHOD(attribute_provider_t, acquire_address, host_t*,
	private_stroke_attribute_t *this, linked_list_t *pools, identification_t *id,
	host_t *requested)
{
	host_t *addr;

	this->lock->read_lock(this->lock);

	addr = find_addr(this, pools, id, requested, MEM_POOL_EXISTING);
	if (!addr)
	{
		addr = find_addr(this, pools, id, requested, MEM_POOL_NEW);
		if (!addr)
		{
			addr = find_addr(this, pools, id, requested, MEM_POOL_REASSIGN);
		}
	}

	this->lock->unlock(this->lock);

	return addr;
}

METHOD(attribute_provider_t, release_address, bool,
	private_stroke_attribute_t *this, linked_list_t *pools, host_t *address,
	identification_t *id)
{
	enumerator_t *enumerator;
	mem_pool_t *pool;
	bool found = FALSE;
	char *name;

	enumerator = pools->create_enumerator(pools);
	this->lock->read_lock(this->lock);
	while (enumerator->enumerate(enumerator, &name))
	{
		pool = find_pool(this, name);
		if (pool)
		{
			found = pool->release_address(pool, address, id);
			if (found)
			{
				break;
			}
		}
	}
	this->lock->unlock(this->lock);
	enumerator->destroy(enumerator);

	return found;
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
	*data = host->get_address(host);
	return TRUE;
}

METHOD(attribute_provider_t, create_attribute_enumerator, enumerator_t*,
	private_stroke_attribute_t *this, linked_list_t *pools,
	identification_t *id, linked_list_t *vips)
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

METHOD(stroke_attribute_t, add_pool, void,
	private_stroke_attribute_t *this, mem_pool_t *pool)
{
	enumerator_t *enumerator;
	mem_pool_t *current;
	host_t *base;
	int size;

	base = pool->get_base(pool);
	size = pool->get_size(pool);

	this->lock->write_lock(this->lock);

	enumerator = this->pools->create_enumerator(this->pools);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (base && current->get_base(current) &&
			base->ip_equals(base, current->get_base(current)) &&
			size == current->get_size(current))
		{
			DBG1(DBG_CFG, "reusing virtual IP address pool %s",
				 current->get_name(current));
			pool->destroy(pool);
			pool = NULL;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (pool)
	{
		if (base)
		{
			DBG1(DBG_CFG, "adding virtual IP address pool %s",
				 pool->get_name(pool));
		}
		this->pools->insert_last(this->pools, pool);
	}

	this->lock->unlock(this->lock);
}

METHOD(stroke_attribute_t, add_dns, void,
	private_stroke_attribute_t *this, stroke_msg_t *msg)
{
	if (msg->add_conn.other.dns)
	{
		enumerator_t *enumerator;
		attributes_t *attr = NULL;
		host_t *host;
		char *token;

		enumerator = enumerator_create_token(msg->add_conn.other.dns, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			host = host_create_from_string(token, 0);
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

METHOD(stroke_attribute_t, del_dns, void,
	private_stroke_attribute_t *this, stroke_msg_t *msg)
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

/**
 * Pool enumerator filter function, converts pool_t to name, size, ...
 */
static bool pool_filter(void *lock, mem_pool_t **poolp, const char **name,
						void *d1, u_int *size, void *d2, u_int *online,
						void *d3, u_int *offline)
{
	mem_pool_t *pool = *poolp;

	if (pool->get_size(pool) == 0)
	{
		return FALSE;
	}
	*name = pool->get_name(pool);
	*size = pool->get_size(pool);
	*online = pool->get_online(pool);
	*offline = pool->get_offline(pool);
	return TRUE;
}

METHOD(stroke_attribute_t, create_pool_enumerator, enumerator_t*,
	private_stroke_attribute_t *this)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_filter(this->pools->create_enumerator(this->pools),
									(void*)pool_filter,
									this->lock, (void*)this->lock->unlock);
}

METHOD(stroke_attribute_t, create_lease_enumerator, enumerator_t*,
	private_stroke_attribute_t *this, char *name)
{
	mem_pool_t *pool;
	this->lock->read_lock(this->lock);
	pool = find_pool(this, name);
	if (!pool)
	{
		this->lock->unlock(this->lock);
		return NULL;
	}
	return enumerator_create_cleaner(pool->create_lease_enumerator(pool),
									 (void*)this->lock->unlock, this->lock);
}

METHOD(stroke_attribute_t, destroy, void,
	private_stroke_attribute_t *this)
{
	this->lock->destroy(this->lock);
	this->pools->destroy_offset(this->pools, offsetof(mem_pool_t, destroy));
	this->attrs->destroy_function(this->attrs, (void*)attributes_destroy);
	free(this);
}

/*
 * see header file
 */
stroke_attribute_t *stroke_attribute_create()
{
	private_stroke_attribute_t *this;

	INIT(this,
		.public = {
			.provider = {
				.acquire_address = _acquire_address,
				.release_address = _release_address,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.add_pool = _add_pool,
			.add_dns = _add_dns,
			.del_dns = _del_dns,
			.create_pool_enumerator = _create_pool_enumerator,
			.create_lease_enumerator = _create_lease_enumerator,
			.destroy = _destroy,
		},
		.pools = linked_list_create(),
		.attrs = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}

