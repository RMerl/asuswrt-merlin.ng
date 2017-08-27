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

#include "unity_handler.h"

#include <daemon.h>
#include <threading/mutex.h>
#include <collections/linked_list.h>
#include <processing/jobs/callback_job.h>

typedef struct private_unity_handler_t private_unity_handler_t;

/**
 * Private data of an unity_handler_t object.
 */
struct private_unity_handler_t {

	/**
	 * Public unity_handler_t interface.
	 */
	unity_handler_t public;

	/**
	 * List of subnets to include, as entry_t
	 */
	linked_list_t *include;

	/**
	 * Mutex for concurrent access to lists
	 */
	mutex_t *mutex;
};

/**
 * Traffic selector entry for networks to include under a given IKE_SA
 */
typedef struct {
	/** associated IKE_SA, unique ID */
	u_int32_t sa;
	/** traffic selector to include/exclude */
	traffic_selector_t *ts;
} entry_t;

/**
 * Clean up an entry
 */
static void entry_destroy(entry_t *this)
{
	this->ts->destroy(this->ts);
	free(this);
}

/**
 * Create a traffic selector from a unity subnet definition
 */
static traffic_selector_t *create_ts(chunk_t subnet)
{
	chunk_t net, mask;
	int i;

	net = chunk_create(subnet.ptr, 4);
	mask = chunk_clonea(chunk_create(subnet.ptr + 4, 4));
	for (i = 0; i < net.len; i++)
	{
		mask.ptr[i] = (mask.ptr[i] ^ 0xFF) | net.ptr[i];
	}
	return traffic_selector_create_from_bytes(0, TS_IPV4_ADDR_RANGE,
											  net, 0, mask, 65535);
}

/**
 * Parse a unity attribute and extract all subnets as traffic selectors
 */
static linked_list_t *parse_subnets(chunk_t data)
{
	linked_list_t *list = NULL;
	traffic_selector_t *ts;

	while (data.len >= 8)
	{	/* the padding is optional */
		ts = create_ts(data);
		if (ts)
		{
			if (!list)
			{
				list = linked_list_create();
			}
			list->insert_last(list, ts);
		}
		/* skip address, mask and 6 bytes of padding */
		data = chunk_skip(data, 14);
	}
	return list;
}

/**
 * Store a list of subnets to include in tunnels under this IKE_SA
 */
static bool add_include(private_unity_handler_t *this, chunk_t data)
{
	traffic_selector_t *ts;
	linked_list_t *list;
	ike_sa_t *ike_sa;
	entry_t *entry;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		return FALSE;
	}
	list = parse_subnets(data);
	if (!list)
	{
		return FALSE;
	}
	while (list->remove_first(list, (void**)&ts) == SUCCESS)
	{
		INIT(entry,
			.sa = ike_sa->get_unique_id(ike_sa),
			.ts = ts,
		);

		this->mutex->lock(this->mutex);
		this->include->insert_last(this->include, entry);
		this->mutex->unlock(this->mutex);
	}
	list->destroy(list);
	return TRUE;
}

/**
 * Remove a list of subnets from the inclusion list for this IKE_SA
 */
static bool remove_include(private_unity_handler_t *this, chunk_t data)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	linked_list_t *list;
	ike_sa_t *ike_sa;
	entry_t *entry;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		return FALSE;
	}
	list = parse_subnets(data);
	if (!list)
	{
		return FALSE;
	}

	this->mutex->lock(this->mutex);
	while (list->remove_first(list, (void**)&ts) == SUCCESS)
	{
		enumerator = this->include->create_enumerator(this->include);
		while (enumerator->enumerate(enumerator, &entry))
		{
			if (entry->sa == ike_sa->get_unique_id(ike_sa) &&
				ts->equals(ts, entry->ts))
			{
				this->include->remove_at(this->include, enumerator);
				entry_destroy(entry);
				break;
			}
		}
		enumerator->destroy(enumerator);
		ts->destroy(ts);
	}
	this->mutex->unlock(this->mutex);
	list->destroy(list);
	return TRUE;
}

/**
 * Create a unique shunt name for a bypass policy
 */
static void create_shunt_name(ike_sa_t *ike_sa, traffic_selector_t *ts,
							  char *buf, size_t len)
{
	snprintf(buf, len, "Unity (%s[%u]: %R)", ike_sa->get_name(ike_sa),
			 ike_sa->get_unique_id(ike_sa), ts);
}

/**
 * Install entry as a shunt policy
 */
static job_requeue_t add_exclude_async(entry_t *entry)
{
	enumerator_t *enumerator;
	child_cfg_t *child_cfg;
	lifetime_cfg_t lft = { .time = { .life = 0 } };
	ike_sa_t *ike_sa;
	char name[128];
	host_t *host;

	ike_sa = charon->ike_sa_manager->checkout_by_id(charon->ike_sa_manager,
													entry->sa, FALSE);
	if (ike_sa)
	{
		create_shunt_name(ike_sa, entry->ts, name, sizeof(name));

		child_cfg = child_cfg_create(name, &lft, NULL, TRUE, MODE_PASS,
									 ACTION_NONE, ACTION_NONE, ACTION_NONE,
									 FALSE, 0, 0, NULL, NULL, FALSE);
		child_cfg->add_traffic_selector(child_cfg, FALSE,
										entry->ts->clone(entry->ts));
		host = ike_sa->get_my_host(ike_sa);
		child_cfg->add_traffic_selector(child_cfg, TRUE,
				traffic_selector_create_from_subnet(host->clone(host),
													32, 0, 0, 65535));
		enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, TRUE);
		while (enumerator->enumerate(enumerator, &host))
		{
			child_cfg->add_traffic_selector(child_cfg, TRUE,
				traffic_selector_create_from_subnet(host->clone(host),
													32, 0, 0, 65535));
		}
		enumerator->destroy(enumerator);
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);

		charon->shunts->install(charon->shunts, child_cfg);
		child_cfg->destroy(child_cfg);

		DBG1(DBG_IKE, "installed %N bypass policy for %R",
			 configuration_attribute_type_names, UNITY_LOCAL_LAN, entry->ts);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Add a bypass policy for a given subnet
 */
static bool add_exclude(private_unity_handler_t *this, chunk_t data)
{
	traffic_selector_t *ts;
	linked_list_t *list;
	ike_sa_t *ike_sa;
	entry_t *entry;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		return FALSE;
	}
	list = parse_subnets(data);
	if (!list)
	{
		return FALSE;
	}

	while (list->remove_first(list, (void**)&ts) == SUCCESS)
	{
		INIT(entry,
			.sa = ike_sa->get_unique_id(ike_sa),
			.ts = ts,
		);

		/* we can't install the shunt policy yet, as we don't know the virtual IP.
		 * Defer installation using an async callback. */
		lib->processor->queue_job(lib->processor, (job_t*)
							callback_job_create((void*)add_exclude_async, entry,
												(void*)entry_destroy, NULL));
	}
	list->destroy(list);
	return TRUE;
}

/**
 * Remove a bypass policy for a given subnet
 */
static bool remove_exclude(private_unity_handler_t *this, chunk_t data)
{
	traffic_selector_t *ts;
	linked_list_t *list;
	ike_sa_t *ike_sa;
	char name[128];
	bool success = TRUE;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		return FALSE;
	}
	list = parse_subnets(data);
	if (!list)
	{
		return FALSE;
	}
	while (list->remove_first(list, (void**)&ts) == SUCCESS)
	{
		create_shunt_name(ike_sa, ts, name, sizeof(name));
		DBG1(DBG_IKE, "uninstalling %N bypass policy for %R",
			 configuration_attribute_type_names, UNITY_LOCAL_LAN, ts);
		ts->destroy(ts);
		success = charon->shunts->uninstall(charon->shunts, name) && success;
	}
	list->destroy(list);
	return success;
}

METHOD(attribute_handler_t, handle, bool,
	private_unity_handler_t *this, identification_t *id,
	configuration_attribute_type_t type, chunk_t data)
{
	switch (type)
	{
		case UNITY_SPLIT_INCLUDE:
			return add_include(this, data);
		case UNITY_LOCAL_LAN:
			return add_exclude(this, data);
		default:
			return FALSE;
	}
}

METHOD(attribute_handler_t, release, void,
	private_unity_handler_t *this, identification_t *server,
	configuration_attribute_type_t type, chunk_t data)
{
	switch (type)
	{
		case UNITY_SPLIT_INCLUDE:
			remove_include(this, data);
			break;
		case UNITY_LOCAL_LAN:
			remove_exclude(this, data);
			break;
		default:
			break;
	}
}

/**
 * Configuration attributes to request
 */
static configuration_attribute_type_t attributes[] = {
	UNITY_SPLIT_INCLUDE,
	UNITY_LOCAL_LAN,
};

/**
 * Attribute enumerator implementation
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** position in attributes[] */
	int i;
} attribute_enumerator_t;

METHOD(enumerator_t, enumerate_attributes, bool,
	attribute_enumerator_t *this, configuration_attribute_type_t *type,
	chunk_t *data)
{
	if (this->i < countof(attributes))
	{
		*type = attributes[this->i++];
		*data = chunk_empty;
		return TRUE;
	}
	return FALSE;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t *,
	unity_handler_t *this, identification_t *id, linked_list_t *vips)
{
	attribute_enumerator_t *enumerator;
	ike_sa_t *ike_sa;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa || ike_sa->get_version(ike_sa) != IKEV1 ||
		!ike_sa->supports_extension(ike_sa, EXT_CISCO_UNITY))
	{
		return enumerator_create_empty();
	}
	INIT(enumerator,
		.public = {
			.enumerate = (void*)_enumerate_attributes,
			.destroy = (void*)free,
		},
	);
	return &enumerator->public;
}

typedef struct {
	/** mutex to unlock */
	mutex_t *mutex;
	/** IKE_SA ID to filter for */
	u_int32_t id;
} include_filter_t;

/**
 * Include enumerator filter function
 */
static bool include_filter(include_filter_t *data,
						   entry_t **entry, traffic_selector_t **ts)
{
	if ((*entry)->sa == data->id)
	{
		*ts = (*entry)->ts;
		return TRUE;
	}
	return FALSE;
}

/**
 * Destroy include filter data, unlock mutex
 */
static void destroy_filter(include_filter_t *data)
{
	data->mutex->unlock(data->mutex);
	free(data);
}

METHOD(unity_handler_t, create_include_enumerator, enumerator_t*,
	private_unity_handler_t *this, u_int32_t id)
{
	include_filter_t *data;

	INIT(data,
		.mutex = this->mutex,
		.id = id,
	);
	data->mutex->lock(data->mutex);
	return enumerator_create_filter(
					this->include->create_enumerator(this->include),
					(void*)include_filter, data, (void*)destroy_filter);
}

METHOD(unity_handler_t, destroy, void,
	private_unity_handler_t *this)
{
	this->include->destroy(this->include);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
unity_handler_t *unity_handler_create()
{
	private_unity_handler_t *this;

	INIT(this,
		.public = {
			.handler = {
				.handle = _handle,
				.release = _release,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.create_include_enumerator = _create_include_enumerator,
			.destroy = _destroy,
		},
		.include = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	return &this->public;
}
