/*
 * Copyright (C) 2010 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
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

#include "attr_provider.h"

#include <time.h>

#include <hydra.h>
#include <utils/debug.h>
#include <collections/linked_list.h>
#include <threading/rwlock.h>

#define SERVER_MAX		2

typedef struct private_attr_provider_t private_attr_provider_t;
typedef struct attribute_entry_t attribute_entry_t;

/**
 * private data of attr_provider
 */
struct private_attr_provider_t {

	/**
	 * public functions
	 */
	attr_provider_t public;

	/**
	 * List of attributes, attribute_entry_t
	 */
	linked_list_t *attributes;

	/**
	 * Lock for attribute list
	 */
	rwlock_t *lock;
};

struct attribute_entry_t {
	/** type of attribute */
	configuration_attribute_type_t type;
	/** attribute value */
	chunk_t value;
};

/**
 * Destroy an entry
 */
static void attribute_destroy(attribute_entry_t *this)
{
	free(this->value.ptr);
	free(this);
}

/**
 * convert enumerator value from attribute_entry
 */
static bool attr_enum_filter(void *null, attribute_entry_t **in,
			configuration_attribute_type_t *type, void* none, chunk_t *value)
{
	*type = (*in)->type;
	*value = (*in)->value;
	return TRUE;
}

METHOD(attribute_provider_t, create_attribute_enumerator, enumerator_t*,
	private_attr_provider_t *this, linked_list_t *pools,
	identification_t *id, linked_list_t *vips)
{
	if (vips->get_count(vips))
	{
		this->lock->read_lock(this->lock);
		return enumerator_create_filter(
				this->attributes->create_enumerator(this->attributes),
				(void*)attr_enum_filter, this->lock, (void*)this->lock->unlock);
	}
	return enumerator_create_empty();
}

METHOD(attr_provider_t, destroy, void,
	private_attr_provider_t *this)
{
	this->attributes->destroy_function(this->attributes,
									   (void*)attribute_destroy);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * Add an attribute entry to the list
 */
static void add_legacy_entry(private_attr_provider_t *this, char *key, int nr,
							 configuration_attribute_type_t type)
{
	attribute_entry_t *entry;
	host_t *host;
	char *str;

	str = lib->settings->get_str(lib->settings, "%s.%s%d", NULL, lib->ns,
								 key, nr);
	if (str)
	{
		host = host_create_from_string(str, 0);
		if (host)
		{
			entry = malloc_thing(attribute_entry_t);

			if (host->get_family(host) == AF_INET6)
			{
				switch (type)
				{
					case INTERNAL_IP4_DNS:
						type = INTERNAL_IP6_DNS;
						break;
					case INTERNAL_IP4_NBNS:
						type = INTERNAL_IP6_NBNS;
						break;
					default:
						break;
				}
			}
			entry->type = type;
			entry->value = chunk_clone(host->get_address(host));
			host->destroy(host);
			DBG2(DBG_CFG, "loaded legacy entry attribute %N: %#B",
				 configuration_attribute_type_names, entry->type, &entry->value);
			this->attributes->insert_last(this->attributes, entry);
		}
	}
}

/**
 * Key to attribute type mappings, for v4 and v6 attributes
 */
typedef struct {
	char *name;
	configuration_attribute_type_t v4;
	configuration_attribute_type_t v6;
} attribute_type_key_t;

static attribute_type_key_t keys[] = {
	{"address",			INTERNAL_IP4_ADDRESS,	INTERNAL_IP6_ADDRESS},
	{"dns",				INTERNAL_IP4_DNS,		INTERNAL_IP6_DNS},
	{"nbns",			INTERNAL_IP4_NBNS,		INTERNAL_IP6_NBNS},
	{"dhcp",			INTERNAL_IP4_DHCP,		INTERNAL_IP6_DHCP},
	{"netmask",			INTERNAL_IP4_NETMASK,	INTERNAL_IP6_NETMASK},
	{"server",			INTERNAL_IP4_SERVER,	INTERNAL_IP6_SERVER},
	{"subnet",			INTERNAL_IP4_SUBNET,	INTERNAL_IP6_SUBNET},
	{"split-include",	UNITY_SPLIT_INCLUDE,	UNITY_SPLIT_INCLUDE},
	{"split-exclude",	UNITY_LOCAL_LAN,		UNITY_LOCAL_LAN},
};

/**
 * Load (numerical) entries from the plugins.attr namespace
 */
static void load_entries(private_attr_provider_t *this)
{
	enumerator_t *enumerator, *tokens;
	char *key, *value, *token;
	int i;

	for (i = 1; i <= SERVER_MAX; i++)
	{
		add_legacy_entry(this, "dns", i, INTERNAL_IP4_DNS);
		add_legacy_entry(this, "nbns", i, INTERNAL_IP4_NBNS);
	}

	enumerator = lib->settings->create_key_value_enumerator(lib->settings,
													"%s.plugins.attr", lib->ns);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		configuration_attribute_type_t type;
		attribute_type_key_t *mapped = NULL;
		attribute_entry_t *entry;
		chunk_t data;
		host_t *host;
		char *pos;
		int i, mask = -1, family;

		if (streq(key, "load"))
		{
			continue;
		}
		type = atoi(key);
		if (!type)
		{
			for (i = 0; i < countof(keys); i++)
			{
				if (streq(key, keys[i].name))
				{
					mapped = &keys[i];
					break;
				}
			}
			if (!mapped)
			{
				DBG1(DBG_CFG, "mapping attribute type %s failed", key);
				continue;
			}
		}
		tokens = enumerator_create_token(value, ",", " ");
		while (tokens->enumerate(tokens, &token))
		{
			pos = strchr(token, '/');
			if (pos)
			{
				*(pos++) = '\0';
				mask = atoi(pos);
			}
			host = host_create_from_string(token, 0);
			if (!host)
			{
				if (mapped)
				{
					DBG1(DBG_CFG, "invalid host in key %s: %s", key, token);
					continue;
				}
				/* store numeric attributes that are no IP addresses as strings */
				data = chunk_clone(chunk_from_str(token));
			}
			else
			{
				family = host->get_family(host);
				if (mask == -1)
				{
					data = chunk_clone(host->get_address(host));
				}
				else
				{
					if (family == AF_INET)
					{	/* IPv4 attributes contain a subnet mask */
						u_int32_t netmask = 0;

						if (mask)
						{	/* shifting u_int32_t by 32 or more is undefined */
							mask = 32 - mask;
							netmask = htonl((0xFFFFFFFF >> mask) << mask);
						}
						data = chunk_cat("cc", host->get_address(host),
										 chunk_from_thing(netmask));
					}
					else
					{	/* IPv6 addresses the prefix only */
						data = chunk_cat("cc", host->get_address(host),
										 chunk_from_chars(mask));
					}
				}
				host->destroy(host);
				if (mapped)
				{
					switch (family)
					{
						case AF_INET:
							type = mapped->v4;
							break;
						case AF_INET6:
							type = mapped->v6;
							break;
					}
				}
			}
			INIT(entry,
				.type = type,
				.value = data,
			);
			DBG2(DBG_CFG, "loaded attribute %N: %#B",
				 configuration_attribute_type_names, entry->type, &entry->value);
			this->attributes->insert_last(this->attributes, entry);
		}
		tokens->destroy(tokens);
	}
	enumerator->destroy(enumerator);
}

METHOD(attr_provider_t, reload, void,
	private_attr_provider_t *this)
{
	this->lock->write_lock(this->lock);

	this->attributes->destroy_function(this->attributes, (void*)attribute_destroy);
	this->attributes = linked_list_create();

	load_entries(this);

	DBG1(DBG_CFG, "loaded %d entr%s for attr plugin configuration",
		 this->attributes->get_count(this->attributes),
		 this->attributes->get_count(this->attributes) == 1 ? "y" : "ies");

	this->lock->unlock(this->lock);
}

/*
 * see header file
 */
attr_provider_t *attr_provider_create(database_t *db)
{
	private_attr_provider_t *this;

	INIT(this,
		.public = {
			.provider = {
				.acquire_address = (void*)return_null,
				.release_address = (void*)return_false,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.reload = _reload,
			.destroy = _destroy,
		},
		.attributes = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	load_entries(this);

	return &this->public;
}
