/*
 * Copyright (C) 2010 Tobias Brunner
 * Copyright (C) 2008-2010 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#include "mem_pool.h"

#include <library.h>
#include <utils/debug.h>
#include <collections/hashtable.h>
#include <collections/array.h>
#include <threading/mutex.h>

#define POOL_LIMIT (sizeof(u_int)*8 - 1)

typedef struct private_mem_pool_t private_mem_pool_t;

/**
 * private data of mem_pool_t
 */
struct private_mem_pool_t {
	/**
	 * public interface
	 */
	mem_pool_t public;

	/**
	 * name of the pool
	 */
	char *name;

	/**
	 * base address of the pool
	 */
	host_t *base;

	/**
	 * whether base is the network id of the subnet on which the pool is based
	 */
	bool base_is_network_id;

	/**
	 * size of the pool
	 */
	u_int size;

	/**
	 * next unused address
	 */
	u_int unused;

	/**
	 * lease hashtable [identity => entry]
	 */
	hashtable_t *leases;

	/**
	 * lock to safely access the pool
	 */
	mutex_t *mutex;
};

/**
 * A unique lease address offset, with a hash of the peer host address
 */
typedef struct {
	/** lease, as offset */
	u_int offset;
	/** hash of remote address, to allow duplicates */
	u_int hash;
} unique_lease_t;

/**
 * Lease entry.
 */
typedef struct {
	/* identitiy reference */
	identification_t *id;
	/* array of online leases, as unique_lease_t */
	array_t *online;
	/* array of offline leases, as u_int offset */
	array_t *offline;
} entry_t;

/**
 * Create a new entry
 */
static entry_t* entry_create(identification_t *id)
{
	entry_t *entry;

	INIT(entry,
		.id = id->clone(id),
		.online = array_create(sizeof(unique_lease_t), 0),
		.offline = array_create(sizeof(u_int), 0),
	);
	return entry;
}

/**
 * Destroy an entry
 */
static void entry_destroy(entry_t *this)
{
	this->id->destroy(this->id);
	array_destroy(this->online);
	array_destroy(this->offline);
	free(this);
}

/**
 * hashtable hash function for identities
 */
static u_int id_hash(identification_t *id)
{
	return chunk_hash(id->get_encoding(id));
}

/**
 * hashtable equals function for identities
 */
static bool id_equals(identification_t *a, identification_t *b)
{
	return a->equals(a, b);
}

/**
 * convert a pool offset to an address
 */
static host_t* offset2host(private_mem_pool_t *pool, int offset)
{
	chunk_t addr;
	host_t *host;
	uint32_t *pos;

	offset--;
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
static int host2offset(private_mem_pool_t *pool, host_t *addr)
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
	return hosti - basei + 1;
}

METHOD(mem_pool_t, get_name, const char*,
	private_mem_pool_t *this)
{
	return this->name;
}

METHOD(mem_pool_t, get_base, host_t*,
	private_mem_pool_t *this)
{
	return this->base;
}

METHOD(mem_pool_t, get_size, u_int,
	private_mem_pool_t *this)
{
	return this->size;
}

METHOD(mem_pool_t, get_online, u_int,
	private_mem_pool_t *this)
{
	enumerator_t *enumerator;
	entry_t *entry;
	u_int count = 0;

	this->mutex->lock(this->mutex);
	enumerator = this->leases->create_enumerator(this->leases);
	while (enumerator->enumerate(enumerator, NULL, &entry))
	{
		count += array_count(entry->online);
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);

	return count;
}

METHOD(mem_pool_t, get_offline, u_int,
	private_mem_pool_t *this)
{
	enumerator_t *enumerator;
	entry_t *entry;
	u_int count = 0;

	this->mutex->lock(this->mutex);
	enumerator = this->leases->create_enumerator(this->leases);
	while (enumerator->enumerate(enumerator, NULL, &entry))
	{
		count += array_count(entry->offline);
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);

	return count;
}

/**
 * Create a unique hash for a remote address
 */
static u_int hash_addr(host_t *addr)
{
	if (addr)
	{
		return chunk_hash_inc(addr->get_address(addr), addr->get_port(addr));
	}
	return 0;
}

/**
 * Get an existing lease for id
 */
static int get_existing(private_mem_pool_t *this, identification_t *id,
						host_t *requested, host_t *peer)
{
	enumerator_t *enumerator;
	unique_lease_t *lease, reassign;
	u_int *current;
	entry_t *entry;
	int offset = 0;

	entry = this->leases->get(this->leases, id);
	if (!entry)
	{
		return 0;
	}

	/* check for a valid offline lease, refresh */
	enumerator = array_create_enumerator(entry->offline);
	if (enumerator->enumerate(enumerator, &current))
	{
		reassign.offset = offset = *current;
		reassign.hash = hash_addr(peer);
		array_insert(entry->online, ARRAY_TAIL, &reassign);
		array_remove_at(entry->offline, enumerator);
	}
	enumerator->destroy(enumerator);
	if (offset)
	{
		DBG1(DBG_CFG, "reassigning offline lease to '%Y'", id);
		return offset;
	}
	if (!peer)
	{
		return 0;
	}
	/* check for a valid online lease to reassign */
	enumerator = array_create_enumerator(entry->online);
	while (enumerator->enumerate(enumerator, &lease))
	{
		if (lease->offset == host2offset(this, requested) &&
			lease->hash == hash_addr(peer))
		{
			offset = lease->offset;
			/* add an additional "online" entry */
			array_insert(entry->online, ARRAY_TAIL, lease);
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (offset)
	{
		DBG1(DBG_CFG, "reassigning online lease to '%Y'", id);
	}
	return offset;
}

/**
 * Get a new lease for id
 */
static int get_new(private_mem_pool_t *this, identification_t *id, host_t *peer)
{
	entry_t *entry;
	unique_lease_t lease = {};

	if (this->unused < this->size)
	{
		entry = this->leases->get(this->leases, id);
		if (!entry)
		{
			entry = entry_create(id);
			this->leases->put(this->leases, entry->id, entry);
		}
		/* assigning offset, starting by 1 */
		lease.offset = ++this->unused + (this->base_is_network_id ? 1 : 0);
		lease.hash = hash_addr(peer);
		array_insert(entry->online, ARRAY_TAIL, &lease);
		DBG1(DBG_CFG, "assigning new lease to '%Y'", id);
	}
	return lease.offset;
}

/**
 * Get a reassigned lease for id in case the pool is full
 */
static int get_reassigned(private_mem_pool_t *this, identification_t *id,
						  host_t *peer)
{
	enumerator_t *enumerator;
	entry_t *entry;
	u_int current;
	unique_lease_t lease = {};

	enumerator = this->leases->create_enumerator(this->leases);
	while (enumerator->enumerate(enumerator, NULL, &entry))
	{
		if (array_remove(entry->offline, ARRAY_HEAD, &current))
		{
			lease.offset = current;
			DBG1(DBG_CFG, "reassigning existing offline lease by '%Y' "
				 "to '%Y'", entry->id, id);
		}
		if (!array_count(entry->online) && !array_count(entry->offline))
		{
			this->leases->remove_at(this->leases, enumerator);
			entry_destroy(entry);
		}
		if (lease.offset)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (lease.offset)
	{
		entry = this->leases->get(this->leases, id);
		if (!entry)
		{
			entry = entry_create(id);
			this->leases->put(this->leases, entry->id, entry);
		}
		lease.hash = hash_addr(peer);
		array_insert(entry->online, ARRAY_TAIL, &lease);
	}
	return lease.offset;
}

METHOD(mem_pool_t, acquire_address, host_t*,
	private_mem_pool_t *this, identification_t *id, host_t *requested,
	mem_pool_op_t operation, host_t *peer)
{
	int offset = 0;

	/* if the pool is empty (e.g. in the %config case) we simply return the
	 * requested address */
	if (this->size == 0)
	{
		return requested->clone(requested);
	}

	if (requested->get_family(requested) !=
		this->base->get_family(this->base))
	{
		return NULL;
	}

	this->mutex->lock(this->mutex);
	switch (operation)
	{
		case MEM_POOL_EXISTING:
			offset = get_existing(this, id, requested, peer);
			break;
		case MEM_POOL_NEW:
			offset = get_new(this, id, peer);
			break;
		case MEM_POOL_REASSIGN:
			offset = get_reassigned(this, id, peer);
			if (!offset)
			{
				DBG1(DBG_CFG, "pool '%s' is full, unable to assign address",
					 this->name);
			}
			break;
		default:
			break;
	}
	this->mutex->unlock(this->mutex);

	if (offset)
	{
		return offset2host(this, offset);
	}
	return NULL;
}

METHOD(mem_pool_t, release_address, bool,
	private_mem_pool_t *this, host_t *address, identification_t *id)
{
	enumerator_t *enumerator;
	bool found = FALSE, more = FALSE;
	entry_t *entry;
	u_int offset;
	unique_lease_t *current;

	if (this->size != 0)
	{
		this->mutex->lock(this->mutex);
		entry = this->leases->get(this->leases, id);
		if (entry)
		{
			offset = host2offset(this, address);

			enumerator = array_create_enumerator(entry->online);
			while (enumerator->enumerate(enumerator, &current))
			{
				if (current->offset == offset)
				{
					if (!found)
					{	/* remove the first entry only */
						array_remove_at(entry->online, enumerator);
						found = TRUE;
					}
					else
					{	/* but check for more entries */
						more = TRUE;
						break;
					}
				}
			}
			enumerator->destroy(enumerator);

			if (found && !more)
			{
				/* no tunnels are online anymore for this lease, make offline */
				array_insert(entry->offline, ARRAY_TAIL, &offset);
				DBG1(DBG_CFG, "lease %H by '%Y' went offline", address, id);
			}
		}
		this->mutex->unlock(this->mutex);
	}
	return found;
}

/**
 * lease enumerator
 */
typedef struct {
	/** implemented enumerator interface */
	enumerator_t public;
	/** hash-table enumerator */
	enumerator_t *entries;
	/** online enumerator */
	enumerator_t *online;
	/** offline enumerator */
	enumerator_t *offline;
	/** enumerated pool */
	private_mem_pool_t *pool;
	/** currently enumerated entry */
	entry_t *entry;
	/** currently enumerated lease address */
	host_t *addr;
} lease_enumerator_t;

METHOD(enumerator_t, lease_enumerate, bool,
	lease_enumerator_t *this, va_list args)
{
	identification_t **id;
	unique_lease_t *lease;
	host_t **addr;
	u_int *offset;
	bool *online;

	VA_ARGS_VGET(args, id, addr, online);

	DESTROY_IF(this->addr);
	this->addr = NULL;

	while (TRUE)
	{
		if (this->entry)
		{
			if (this->online->enumerate(this->online, &lease))
			{
				*id = this->entry->id;
				*addr = this->addr = offset2host(this->pool, lease->offset);
				*online = TRUE;
				return TRUE;
			}
			if (this->offline->enumerate(this->offline, &offset))
			{
				*id = this->entry->id;
				*addr = this->addr = offset2host(this->pool, *offset);
				*online = FALSE;
				return TRUE;
			}
			this->online->destroy(this->online);
			this->offline->destroy(this->offline);
			this->online = this->offline = NULL;
		}
		if (!this->entries->enumerate(this->entries, NULL, &this->entry))
		{
			return FALSE;
		}
		this->online = array_create_enumerator(this->entry->online);
		this->offline = array_create_enumerator(this->entry->offline);
	}
}

METHOD(enumerator_t, lease_enumerator_destroy, void,
	lease_enumerator_t *this)
{
	DESTROY_IF(this->addr);
	DESTROY_IF(this->online);
	DESTROY_IF(this->offline);
	this->entries->destroy(this->entries);
	this->pool->mutex->unlock(this->pool->mutex);
	free(this);
}

METHOD(mem_pool_t, create_lease_enumerator, enumerator_t*,
	   private_mem_pool_t *this)
{
	lease_enumerator_t *enumerator;

	this->mutex->lock(this->mutex);
	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _lease_enumerate,
			.destroy = _lease_enumerator_destroy,
		},
		.pool = this,
		.entries = this->leases->create_enumerator(this->leases),
	);
	return &enumerator->public;
}

METHOD(mem_pool_t, destroy, void,
	private_mem_pool_t *this)
{
	enumerator_t *enumerator;
	entry_t *entry;

	enumerator = this->leases->create_enumerator(this->leases);
	while (enumerator->enumerate(enumerator, NULL, &entry))
	{
		entry_destroy(entry);
	}
	enumerator->destroy(enumerator);

	this->leases->destroy(this->leases);
	this->mutex->destroy(this->mutex);
	DESTROY_IF(this->base);
	free(this->name);
	free(this);
}

/**
 * Generic constructor
 */
static private_mem_pool_t *create_generic(char *name)
{
	private_mem_pool_t *this;

	INIT(this,
		.public = {
			.get_name = _get_name,
			.get_base = _get_base,
			.get_size = _get_size,
			.get_online = _get_online,
			.get_offline = _get_offline,
			.acquire_address = _acquire_address,
			.release_address = _release_address,
			.create_lease_enumerator = _create_lease_enumerator,
			.destroy = _destroy,
		},
		.name = strdup(name),
		.leases = hashtable_create((hashtable_hash_t)id_hash,
								   (hashtable_equals_t)id_equals, 16),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	return this;
}

/**
 * Check if the given host is the network ID of a subnet, that is, if hostbits
 * are zero.  Since we limit pools to 2^31 addresses we only have to check the
 * last 4 bytes.
 */
static u_int network_id_diff(host_t *host, int hostbits)
{
	uint32_t last;
	chunk_t addr;

	if (!hostbits)
	{
		return 0;
	}
	addr = host->get_address(host);
	last = untoh32(addr.ptr + addr.len - sizeof(last));
	hostbits = sizeof(last) * 8 - hostbits;
	return (last << hostbits) >> hostbits;
}

/**
 * Described in header
 */
mem_pool_t *mem_pool_create(char *name, host_t *base, int bits)
{
	private_mem_pool_t *this;
	u_int diff;
	int addr_bits;

	this = create_generic(name);
	if (base)
	{
		addr_bits = base->get_family(base) == AF_INET ? 32 : 128;
		bits = max(0, min(bits, addr_bits));
		/* net bits -> host bits */
		bits = addr_bits - bits;
		if (bits > POOL_LIMIT)
		{
			bits = POOL_LIMIT;
			DBG1(DBG_CFG, "virtual IP pool too large, limiting to %H/%d",
				 base, addr_bits - bits);
		}
		this->size = 1 << bits;
		this->base = base->clone(base);

		if (this->size > 2)
		{
			/* if base is the network id we later skip the first address,
			 * otherwise adjust the size to represent the actual number
			 * of assignable addresses */
			diff = network_id_diff(base, bits);
			if (!diff)
			{
				this->base_is_network_id = TRUE;
				this->size--;
			}
			else
			{
				this->size -= diff;
			}
			/* skip the last address (broadcast) of the subnet */
			this->size--;
		}
		else if (network_id_diff(base, bits))
		{	/* only serve the second address of the subnet */
			this->size--;
		}
	}
	return &this->public;
}

/**
 * Described in header
 */
mem_pool_t *mem_pool_create_range(char *name, host_t *from, host_t *to)
{
	private_mem_pool_t *this;
	chunk_t fromaddr, toaddr;
	uint32_t diff;

	fromaddr = from->get_address(from);
	toaddr = to->get_address(to);

	if (from->get_family(from) != to->get_family(to) ||
		fromaddr.len != toaddr.len || fromaddr.len < sizeof(diff) ||
		memcmp(fromaddr.ptr, toaddr.ptr, toaddr.len) > 0)
	{
		DBG1(DBG_CFG, "invalid IP address range: %H-%H", from, to);
		return NULL;
	}
	if (fromaddr.len > sizeof(diff) &&
		!chunk_equals(chunk_create(fromaddr.ptr, fromaddr.len - sizeof(diff)),
					  chunk_create(toaddr.ptr, toaddr.len - sizeof(diff))))
	{
		DBG1(DBG_CFG, "IP address range too large: %H-%H", from, to);
		return NULL;
	}
	this = create_generic(name);
	this->base = from->clone(from);
	diff = untoh32(toaddr.ptr + toaddr.len - sizeof(diff)) -
		   untoh32(fromaddr.ptr + fromaddr.len - sizeof(diff));
	this->size = diff + 1;

	return &this->public;
}
