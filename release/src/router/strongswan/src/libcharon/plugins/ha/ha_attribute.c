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

#include "ha_attribute.h"

#include <collections/linked_list.h>
#include <threading/mutex.h>

typedef struct private_ha_attribute_t private_ha_attribute_t;

/**
 * Private data of an ha_attribute_t object.
 */
struct private_ha_attribute_t {

	/**
	 * Public ha_attribute_t interface.
	 */
	ha_attribute_t public;

	/**
	 * List of pools, pool_t
	 */
	linked_list_t *pools;

	/**
	 * Mutex to lock mask
	 */
	mutex_t *mutex;

	/**
	 * Kernel helper
	 */
	ha_kernel_t *kernel;

	/**
	 * Segment responsibility
	 */
	ha_segments_t *segments;
};

/**
 * In-memory pool.
 */
typedef struct {
	/** name of the pool */
	char *name;
	/** base address of pool */
	host_t *base;
	/** total number of addresses in this pool */
	int size;
	/** bitmask for address usage */
	u_char *mask;
} pool_t;

/**
 * Clean up a pool entry
 */
static void pool_destroy(pool_t *pool)
{
	pool->base->destroy(pool->base);
	free(pool->name);
	free(pool->mask);
	free(pool);
}

/**
 * convert a pool offset to an address
 */
static host_t* offset2host(pool_t *pool, int offset)
{
	chunk_t addr;
	host_t *host;
	uint32_t *pos;

	if (offset > pool->size)
	{
		return NULL;
	}

	addr = chunk_clone(pool->base->get_address(pool->base));
	if (pool->base->get_family(pool->base) == AF_INET6)
	{
		pos = (uint32_t*)(addr.ptr + 12);
	}
	else
	{
		pos = (uint32_t*)addr.ptr;
	}
	*pos = htonl(offset + ntohl(*pos));
	host = host_create_from_chunk(pool->base->get_family(pool->base), addr, 0);
	free(addr.ptr);
	return host;
}

/**
 * convert a host to a pool offset
 */
static int host2offset(pool_t *pool, host_t *addr)
{
	chunk_t host, base;
	uint32_t hosti, basei;

	if (addr->get_family(addr) != pool->base->get_family(pool->base))
	{
		return -1;
	}
	host = addr->get_address(addr);
	base = pool->base->get_address(pool->base);
	if (addr->get_family(addr) == AF_INET6)
	{
		/* only look at last /32 block */
		if (!memeq(host.ptr, base.ptr, 12))
		{
			return -1;
		}
		host = chunk_skip(host, 12);
		base = chunk_skip(base, 12);
	}
	hosti = ntohl(*(uint32_t*)(host.ptr));
	basei = ntohl(*(uint32_t*)(base.ptr));
	if (hosti > basei + pool->size)
	{
		return -1;
	}
	return hosti - basei;
}

/**
 * Find a pool by its name
 */
static pool_t* get_pool(private_ha_attribute_t *this, char *name)
{
	enumerator_t *enumerator;
	pool_t *pool, *found = NULL;

	enumerator = this->pools->create_enumerator(this->pools);
	while (enumerator->enumerate(enumerator, &pool))
	{
		if (streq(name, pool->name))
		{
			found = pool;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * Check if we are responsible for an offset
 */
static bool responsible_for(private_ha_attribute_t *this, int offset)
{
	u_int segment;

	segment = offset % this->segments->count(this->segments) + 1;
	return this->segments->is_active(this->segments, segment);
}

METHOD(attribute_provider_t, acquire_address, host_t*,
	private_ha_attribute_t *this, linked_list_t *pools, ike_sa_t *ike_sa,
	host_t *requested)
{
	enumerator_t *enumerator;
	pool_t *pool = NULL;
	int offset = -1, tmp_offset, byte, bit;
	host_t *address;
	char *name;

	enumerator = pools->create_enumerator(pools);
	this->mutex->lock(this->mutex);
	while (enumerator->enumerate(enumerator, &name))
	{
		pool = get_pool(this, name);
		if (!pool)
		{
			continue;
		}
		if (pool->base->get_family(pool->base) !=
			requested->get_family(requested))
		{
			continue;
		}
		for (byte = 0; byte < pool->size / 8; byte++)
		{
			if (pool->mask[byte] != 0xFF)
			{
				for (bit = 0; bit < 8; bit++)
				{
					tmp_offset = byte * 8 + bit;
					if (!(pool->mask[byte] & 1 << bit) &&
						responsible_for(this, tmp_offset))
					{
						offset = tmp_offset;
						pool->mask[byte] |= 1 << bit;
						break;
					}
				}
			}
			if (offset != -1)
			{
				break;
			}
		}
		if (offset == -1)
		{
			DBG1(DBG_CFG, "no address belonging to a responsible segment left "
				 "in HA pool '%s'", name);
		}
		else
		{
			break;
		}
	}
	this->mutex->unlock(this->mutex);
	enumerator->destroy(enumerator);

	if (offset != -1)
	{
		address = offset2host(pool, offset);
		DBG1(DBG_CFG, "acquired address %H from HA pool '%s'", address, name);
		return address;
	}
	return NULL;
}

METHOD(attribute_provider_t, release_address, bool,
	private_ha_attribute_t *this, linked_list_t *pools, host_t *address,
	ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	pool_t *pool;
	int offset;
	char *name;
	bool found = FALSE;

	enumerator = pools->create_enumerator(pools);
	this->mutex->lock(this->mutex);
	while (enumerator->enumerate(enumerator, &name))
	{
		pool = get_pool(this, name);
		if (!pool)
		{
			continue;
		}
		if (pool->base->get_family(pool->base) != address->get_family(address))
		{
			continue;
		}
		offset = host2offset(pool, address);
		if (offset > 0 && offset < pool->size)
		{
			pool->mask[offset / 8] &= ~(1 << (offset % 8));
			DBG1(DBG_CFG, "released address %H to HA pool '%s'", address, name);
			found = TRUE;
			break;
		}
	}
	this->mutex->unlock(this->mutex);
	enumerator->destroy(enumerator);

	return found;
}

METHOD(ha_attribute_t, reserve, void,
	private_ha_attribute_t *this, char *name, host_t *address)
{
	pool_t *pool;
	int offset;

	this->mutex->lock(this->mutex);
	pool = get_pool(this, name);
	if (pool)
	{
		offset = host2offset(pool, address);
		if (offset > 0 && offset < pool->size)
		{
			pool->mask[offset / 8] |= 1 << (offset % 8);
			DBG1(DBG_CFG, "reserved address %H in HA pool '%s'", address, name);
		}
	}
	this->mutex->unlock(this->mutex);
}

METHOD(ha_attribute_t, destroy, void,
	private_ha_attribute_t *this)
{
	this->pools->destroy_function(this->pools, (void*)pool_destroy);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * Load the configured pools.
 */
static void load_pools(private_ha_attribute_t *this)
{
	enumerator_t *enumerator;
	char *name, *net, *bits;
	host_t *base;
	int mask, maxbits;
	pool_t *pool;

	enumerator = lib->settings->create_key_value_enumerator(lib->settings,
												"%s.plugins.ha.pools", lib->ns);
	while (enumerator->enumerate(enumerator, &name, &net))
	{
		net = strdup(net);
		bits = strchr(net, '/');
		if (!bits)
		{
			DBG1(DBG_CFG, "invalid HA pool '%s' subnet, skipped", name);
			free(net);
			continue;
		}
		*bits++ = '\0';

		base = host_create_from_string(net, 0);
		mask = atoi(bits);
		free(net);
		if (!base || !mask)
		{
			DESTROY_IF(base);
			DBG1(DBG_CFG, "invalid HA pool '%s', skipped", name);
			continue;
		}
		maxbits = base->get_family(base) == AF_INET ? 32 : 128;
		mask = maxbits - mask;
		if (mask > 24)
		{
			mask = 24;
			DBG1(DBG_CFG, "size of HA pool '%s' limited to /%d",
				 name, maxbits - mask);
		}
		if (mask < 3)
		{
			DBG1(DBG_CFG, "HA pool '%s' too small, skipped", name);
			base->destroy(base);
			continue;
		}

		INIT(pool,
			.name = strdup(name),
			.base = base,
			.size = (1 << mask),
		);
		pool->mask = calloc(pool->size / 8, 1);
		/* do not use first/last address of pool */
		pool->mask[0] |= 0x01;
		pool->mask[pool->size / 8 - 1] |= 0x80;

		DBG1(DBG_CFG, "loaded HA pool '%s' %H/%d (%d addresses)",
			 pool->name, pool->base, maxbits - mask, pool->size - 2);
		this->pools->insert_last(this->pools, pool);
	}
	enumerator->destroy(enumerator);
}

/**
 * See header
 */
ha_attribute_t *ha_attribute_create(ha_kernel_t *kernel, ha_segments_t *segments)
{
	private_ha_attribute_t *this;

	INIT(this,
		.public = {
			.provider = {
				.acquire_address = _acquire_address,
				.release_address = _release_address,
				.create_attribute_enumerator = enumerator_create_empty,
			},
			.reserve = _reserve,
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.pools = linked_list_create(),
		.kernel = kernel,
		.segments = segments,
	);

	load_pools(this);

	return &this->public;
}
