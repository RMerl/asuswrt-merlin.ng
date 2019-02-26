/*
 * Copyright (C) 2014-2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#include "vici_attribute.h"
#include "vici_builder.h"

#include <daemon.h>
#include <collections/hashtable.h>
#include <collections/array.h>
#include <threading/rwlock.h>
#include <attributes/mem_pool.h>

typedef struct private_vici_attribute_t private_vici_attribute_t;

/**
 * private data of vici_attribute
 */
struct private_vici_attribute_t {

	/**
	 * public functions
	 */
	vici_attribute_t public;

	/**
	 * vici connection dispatcher
	 */
	vici_dispatcher_t *dispatcher;

	/**
	 * Configured pools, as char* => pool_t
	 */
	hashtable_t *pools;

	/**
	 * rwlock to lock access to pools
	 */
	rwlock_t *lock;
};

/**
 * Single configuration attribute with type
 */
typedef struct {
	/** type of attribute */
	configuration_attribute_type_t type;
	/** attribute value */
	chunk_t value;
} attribute_t;

/**
 * Clean up an attribute
 */
static void attribute_destroy(attribute_t *attr)
{
	free(attr->value.ptr);
	free(attr);
}

/**
 * Pool instances with associated attributes
 */
typedef struct {
	/** in-memory virtual IP pool */
	mem_pool_t *vips;
	/** configuration attributes, as attribute_t */
	array_t *attrs;
} pool_t;

/**
 * Clean up a pool instance
 */
static void pool_destroy(pool_t *pool)
{
	DESTROY_IF(pool->vips);
	array_destroy_function(pool->attrs, (void*)attribute_destroy, NULL);
	free(pool);
}

/**
 * Find an existing or not yet existing lease
 */
static host_t *find_addr(private_vici_attribute_t *this, linked_list_t *pools,
						 identification_t *id, host_t *requested,
						 mem_pool_op_t op, host_t *peer)
{
	enumerator_t *enumerator;
	host_t *addr = NULL;
	pool_t *pool;
	char *name;

	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &name))
	{
		pool = this->pools->get(this->pools, name);
		if (pool)
		{
			addr = pool->vips->acquire_address(pool->vips, id, requested,
											   op, peer);
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
	private_vici_attribute_t *this, linked_list_t *pools, ike_sa_t *ike_sa,
	host_t *requested)
{
	identification_t *id;
	host_t *addr, *peer;

	id = ike_sa->get_other_eap_id(ike_sa);
	peer = ike_sa->get_other_host(ike_sa);

	this->lock->read_lock(this->lock);

	addr = find_addr(this, pools, id, requested, MEM_POOL_EXISTING, peer);
	if (!addr)
	{
		addr = find_addr(this, pools, id, requested, MEM_POOL_NEW, peer);
		if (!addr)
		{
			addr = find_addr(this, pools, id, requested, MEM_POOL_REASSIGN, peer);
		}
	}

	this->lock->unlock(this->lock);

	return addr;
}

METHOD(attribute_provider_t, release_address, bool,
	private_vici_attribute_t *this, linked_list_t *pools, host_t *address,
	ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	identification_t *id;
	bool found = FALSE;
	pool_t *pool;
	char *name;

	id = ike_sa->get_other_eap_id(ike_sa);

	this->lock->read_lock(this->lock);

	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &name))
	{
		pool = this->pools->get(this->pools, name);
		if (pool)
		{
			found = pool->vips->release_address(pool->vips, address, id);
			if (found)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);

	this->lock->unlock(this->lock);

	return found;
}

CALLBACK(attr_filter, bool,
	void *data, enumerator_t *orig, va_list args)
{
	attribute_t *attr;
	configuration_attribute_type_t *type;
	chunk_t *value;

	VA_ARGS_VGET(args, type, value);

	if (orig->enumerate(orig, &attr))
	{
		*type = attr->type;
		*value = attr->value;
		return TRUE;
	}
	return FALSE;
}

/**
 * Create nested inner enumerator over pool attributes
 */
CALLBACK(create_nested, enumerator_t*,
	pool_t *pool, void *this)
{
	return enumerator_create_filter(array_create_enumerator(pool->attrs),
									attr_filter, NULL, NULL);
}

/**
 * Data associated to nested enumerator cleanup
 */
typedef struct {
	private_vici_attribute_t *this;
	linked_list_t *list;
} nested_data_t;

/**
 * Clean up nested enumerator data
 */
CALLBACK(nested_cleanup, void,
	nested_data_t *data)
{
	data->this->lock->unlock(data->this->lock);
	data->list->destroy(data->list);
	free(data);
}

/**
 * Check if any of vips is from pool
 */
static bool have_vips_from_pool(mem_pool_t *pool, linked_list_t *vips)
{
	enumerator_t *enumerator;
	host_t *host;
	chunk_t start, end, current;
	uint32_t size;
	bool found = FALSE;

	host = pool->get_base(pool);
	start = host->get_address(host);

	if (start.len >= sizeof(size))
	{
		end = chunk_clone(start);

		/* mem_pool is currently limited to 2^31 addresses, so 32-bit
		 * calculations should be sufficient. */
		size = untoh32(start.ptr + start.len - sizeof(size));
		htoun32(end.ptr + end.len - sizeof(size), size + pool->get_size(pool));

		enumerator = vips->create_enumerator(vips);
		while (enumerator->enumerate(enumerator, &host))
		{
			current = host->get_address(host);
			if (chunk_compare(current, start) >= 0 &&
				chunk_compare(current, end) < 0)
			{
				found = TRUE;
				break;
			}
		}
		enumerator->destroy(enumerator);

		free(end.ptr);
	}
	return found;
}

METHOD(attribute_provider_t, create_attribute_enumerator, enumerator_t*,
	private_vici_attribute_t *this, linked_list_t *pools,
	ike_sa_t *ike_sa, linked_list_t *vips)
{
	enumerator_t *enumerator;
	nested_data_t *data;
	pool_t *pool;
	char *name;

	INIT(data,
		.this = this,
		.list = linked_list_create(),
	);

	this->lock->read_lock(this->lock);

	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &name))
	{
		pool = this->pools->get(this->pools, name);
		if (pool && have_vips_from_pool(pool->vips, vips))
		{
			data->list->insert_last(data->list, pool);
		}
	}
	enumerator->destroy(enumerator);

	return enumerator_create_nested(data->list->create_enumerator(data->list),
									create_nested, data, nested_cleanup);
}

/**
 * Merge a pool configuration with existing ones
 */
static bool merge_pool(private_vici_attribute_t *this, pool_t *new)
{
	mem_pool_t *tmp;
	host_t *base;
	pool_t *old;
	const char *name;
	u_int size;

	name = new->vips->get_name(new->vips);
	base = new->vips->get_base(new->vips);
	size = new->vips->get_size(new->vips);

	old = this->pools->remove(this->pools, name);
	if (!old)
	{
		this->pools->put(this->pools, name, new);
		DBG1(DBG_CFG, "added vici pool %s: %H, %u entries", name, base, size);
		return TRUE;
	}

	if (base->ip_equals(base, old->vips->get_base(old->vips)) &&
		size == old->vips->get_size(old->vips))
	{
		/* no changes in pool, so keep existing, but use new attributes */
		DBG1(DBG_CFG, "updated vici pool %s: %H, %u entries", name, base, size);
		tmp = new->vips;
		new->vips = old->vips;
		old->vips = tmp;
		this->pools->put(this->pools, new->vips->get_name(new->vips), new);
		pool_destroy(old);
		return TRUE;
	}
	if (old->vips->get_online(old->vips) == 0)
	{
		/* can replace old pool, no online leases */
		DBG1(DBG_CFG, "replaced vici pool %s: %H, %u entries", name, base, size);
		this->pools->put(this->pools, name, new);
		pool_destroy(old);
		return TRUE;
	}
	/* have online leases, unable to replace, TODO: migrate leases? */
	DBG1(DBG_CFG, "vici pool %s has %u online leases, unable to replace",
		 name, old->vips->get_online(old->vips));
	this->pools->put(this->pools, old->vips->get_name(old->vips), old);
	return FALSE;
}

/**
 * Create a (error) reply message
 */
static vici_message_t* create_reply(char *fmt, ...)
{
	vici_builder_t *builder;
	va_list args;

	builder = vici_builder_create();
	builder->add_kv(builder, "success", fmt ? "no" : "yes");
	if (fmt)
	{
		va_start(args, fmt);
		builder->vadd_kv(builder, "errmsg", fmt, args);
		va_end(args);
	}
	return builder->finalize(builder);
}

/**
 * Parse a range definition of an address pool
 */
static mem_pool_t *create_pool_range(char *name, char *buf)
{
	mem_pool_t *pool;
	host_t *from, *to;

	if (!host_create_from_range(buf, &from, &to))
	{
		return NULL;
	}
	pool = mem_pool_create_range(name, from, to);
	from->destroy(from);
	to->destroy(to);
	return pool;
}

/**
 * Parse callback data, passed to each callback
 */
typedef struct {
	private_vici_attribute_t *this;
	vici_message_t *reply;
} request_data_t;

/**
 * Data associated to a pool load
 */
typedef struct {
	request_data_t *request;
	char *name;
	pool_t *pool;
} load_data_t;

CALLBACK(pool_li, bool,
	load_data_t *data, vici_message_t *message, char *name, chunk_t value)
{
	struct {
		char *name;
		configuration_attribute_type_t v4;
		configuration_attribute_type_t v6;
	} keys[] = {
		{"address",			INTERNAL_IP4_ADDRESS,	INTERNAL_IP6_ADDRESS	},
		{"dns",				INTERNAL_IP4_DNS,		INTERNAL_IP6_DNS		},
		{"nbns",			INTERNAL_IP4_NBNS,		INTERNAL_IP6_NBNS		},
		{"dhcp",			INTERNAL_IP4_DHCP,		INTERNAL_IP6_DHCP		},
		{"netmask",			INTERNAL_IP4_NETMASK,	INTERNAL_IP6_NETMASK	},
		{"server",			INTERNAL_IP4_SERVER,	INTERNAL_IP6_SERVER		},
		{"subnet",			INTERNAL_IP4_SUBNET,	INTERNAL_IP6_SUBNET		},
		{"split_include",	UNITY_SPLIT_INCLUDE,	UNITY_SPLIT_INCLUDE		},
		{"split_exclude",	UNITY_LOCAL_LAN,		UNITY_LOCAL_LAN			},
	};
	char buf[256];
	int i, index = -1, mask = -1, type = 0;
	chunk_t encoding;
	attribute_t *attr;
	host_t *host = NULL;

	for (i = 0; i < countof(keys); i++)
	{
		if (streq(name, keys[i].name))
		{
			index = i;
			break;
		}
	}
	if (index == -1)
	{
		type = atoi(name);
		if (!type)
		{
			data->request->reply = create_reply("invalid attribute: %s", name);
			return FALSE;
		}
	}

	if (vici_stringify(value, buf, sizeof(buf)))
	{
		if (strchr(buf, '/'))
		{
			host = host_create_from_subnet(buf, &mask);
		}
		else
		{
			host = host_create_from_string(buf, 0);
		}
	}
	if (host)
	{
		if (index != -1)
		{
			switch (host->get_family(host))
			{
				case AF_INET:
					type = keys[index].v4;
					break;
				case AF_INET6:
				default:
					type = keys[index].v6;
					break;
			}
		}
		if (mask == -1)
		{
			encoding = chunk_clone(host->get_address(host));
		}
		else
		{
			if (host->get_family(host) == AF_INET)
			{	/* IPv4 attributes contain a subnet mask */
				uint32_t netmask = 0;

				if (mask)
				{	/* shifting uint32_t by 32 or more is undefined */
					mask = 32 - mask;
					netmask = htonl((0xFFFFFFFF >> mask) << mask);
				}
				encoding = chunk_cat("cc", host->get_address(host),
									 chunk_from_thing(netmask));
			}
			else
			{	/* IPv6 addresses the prefix only */
				encoding = chunk_cat("cc", host->get_address(host),
									 chunk_from_chars(mask));
			}
		}
		host->destroy(host);
	}
	else
	{
		if (index != -1)
		{
			data->request->reply = create_reply("invalid attribute value "
												"for %s", name);
			return FALSE;
		}
		/* use raw binary data for numbered attributes */
		encoding = chunk_clone(value);
	}
	INIT(attr,
		.type = type,
		.value = encoding,
	);
	array_insert_create(&data->pool->attrs, ARRAY_TAIL, attr);
	return TRUE;
}

CALLBACK(pool_kv, bool,
	load_data_t *data, vici_message_t *message, char *name, chunk_t value)
{
	if (streq(name, "addrs"))
	{
		char buf[128];
		mem_pool_t *pool;
		host_t *base = NULL;
		int bits;

		if (data->pool->vips)
		{
			data->request->reply = create_reply("multiple addrs defined");
			return FALSE;
		}
		if (!vici_stringify(value, buf, sizeof(buf)))
		{
			data->request->reply = create_reply("invalid addrs value");
			return FALSE;
		}
		pool = create_pool_range(data->name, buf);
		if (!pool)
		{
			base = host_create_from_subnet(buf, &bits);
			if (base)
			{
				pool = mem_pool_create(data->name, base, bits);
				base->destroy(base);
			}
		}
		if (!pool)
		{
			data->request->reply = create_reply("invalid addrs value: %s", buf);
			return FALSE;
		}
		data->pool->vips = pool;
		return TRUE;
	}
	data->request->reply = create_reply("invalid attribute: %s", name);
	return FALSE;
}

CALLBACK(pool_sn, bool,
	request_data_t *request, vici_message_t *message,
	vici_parse_context_t *ctx, char *name)
{
	load_data_t data = {
		.request = request,
		.name = name,
	};
	bool merged;

	INIT(data.pool);

	if (!message->parse(message, ctx, NULL, pool_kv, pool_li, &data))
	{
		pool_destroy(data.pool);
		return FALSE;
	}

	if (!data.pool->vips)
	{
		request->reply = create_reply("missing addrs for pool '%s'", name);
		pool_destroy(data.pool);
		return FALSE;
	}

	request->this->lock->write_lock(request->this->lock);
	merged = merge_pool(request->this, data.pool);
	request->this->lock->unlock(request->this->lock);

	if (!merged)
	{
		request->reply = create_reply("vici pool %s has online leases, "
									  "unable to replace", name);
		pool_destroy(data.pool);
	}
	return merged;
}

CALLBACK(load_pool, vici_message_t*,
	private_vici_attribute_t *this, char *name, u_int id,
	vici_message_t *message)
{
	request_data_t request = {
		.this = this,
	};

	if (!message->parse(message, NULL, pool_sn, NULL, NULL, &request))
	{
		if (request.reply)
		{
			return request.reply;
		}
		return create_reply("parsing request failed");
	}
	return create_reply(NULL);
}

CALLBACK(unload_pool, vici_message_t*,
	private_vici_attribute_t *this, char *name, u_int id,
	vici_message_t *message)
{
	vici_message_t *reply;
	u_int online;
	pool_t *pool;

	name = message->get_str(message, NULL, "name");
	if (!name)
	{
		return create_reply("missing pool name to unload");
	}

	this->lock->write_lock(this->lock);

	pool = this->pools->remove(this->pools, name);
	if (pool)
	{
		online = pool->vips->get_online(pool->vips);
		if (online)
		{
			DBG1(DBG_CFG, "vici pool %s has %u online leases, unable to unload",
				 name, online);
			reply = create_reply("%s has online leases, unable to unload", name);
			this->pools->put(this->pools, pool->vips->get_name(pool->vips), pool);
		}
		else
		{
			DBG1(DBG_CFG, "unloaded vici pool %s", name);
			reply = create_reply(NULL);
			pool_destroy(pool);
		}
	}
	else
	{
		reply = create_reply("%s not found", name);
	}

	this->lock->unlock(this->lock);

	return reply;
}

CALLBACK(get_pools, vici_message_t*,
	private_vici_attribute_t *this, char *name, u_int id,
	vici_message_t *message)
{
	vici_builder_t *builder;
	enumerator_t *enumerator, *leases;
	mem_pool_t *vips;
	pool_t *pool;
	identification_t *uid;
	host_t *lease;
	bool list_leases, on;
	char buf[32], *filter;
	int i;

	list_leases = message->get_bool(message, FALSE, "leases");
	filter = message->get_str(message, NULL, "name");

	builder = vici_builder_create();

	this->lock->read_lock(this->lock);
	enumerator = this->pools->create_enumerator(this->pools);
	while (enumerator->enumerate(enumerator, &name, &pool))
	{
		if (filter && !streq(name, filter))
		{
			continue;
		}

		vips = pool->vips;

		builder->begin_section(builder, name);

		builder->add_kv(builder, "base", "%H", vips->get_base(vips));
		builder->add_kv(builder, "size", "%u", vips->get_size(vips));
		builder->add_kv(builder, "online", "%u", vips->get_online(vips));
		builder->add_kv(builder, "offline", "%u", vips->get_offline(vips));

		if (list_leases)
		{
			i = 0;
			builder->begin_section(builder, "leases");
			leases = vips->create_lease_enumerator(vips);
			while (leases->enumerate(leases, &uid, &lease, &on))
			{
				snprintf(buf, sizeof(buf), "%d", i++);
				builder->begin_section(builder, buf);
				builder->add_kv(builder, "address", "%H", lease);
				builder->add_kv(builder, "identity", "%Y", uid);
				builder->add_kv(builder, "status", on ? "online" : "offline");
				builder->end_section(builder);
			}
			leases->destroy(leases);
			builder->end_section(builder);
		}
		builder->end_section(builder);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return builder->finalize(builder);
}

static void manage_command(private_vici_attribute_t *this,
						   char *name, vici_command_cb_t cb, bool reg)
{
	this->dispatcher->manage_command(this->dispatcher, name,
									 reg ? cb : NULL, this);
}

/**
 * (Un-)register dispatcher functions
 */
static void manage_commands(private_vici_attribute_t *this, bool reg)
{
	manage_command(this, "load-pool", load_pool, reg);
	manage_command(this, "unload-pool", unload_pool, reg);
	manage_command(this, "get-pools", get_pools, reg);
}

METHOD(vici_attribute_t, destroy, void,
	private_vici_attribute_t *this)
{
	enumerator_t *enumerator;
	pool_t *pool;

	manage_commands(this, FALSE);

	enumerator = this->pools->create_enumerator(this->pools);
	while (enumerator->enumerate(enumerator, NULL, &pool))
	{
		pool_destroy(pool);
	}
	enumerator->destroy(enumerator);
	this->pools->destroy(this->pools);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * see header file
 */
vici_attribute_t *vici_attribute_create(vici_dispatcher_t *dispatcher)
{
	private_vici_attribute_t *this;

	INIT(this,
		.public = {
			.provider = {
				.acquire_address = _acquire_address,
				.release_address = _release_address,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.destroy = _destroy,
		},
		.dispatcher = dispatcher,
		.pools = hashtable_create(hashtable_hash_str, hashtable_equals_str, 4),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	manage_commands(this, TRUE);

	return &this->public;
}
