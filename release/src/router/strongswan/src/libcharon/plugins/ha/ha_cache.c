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

#include "ha_cache.h"

#include <collections/hashtable.h>
#include <collections/linked_list.h>
#include <threading/mutex.h>
#include <processing/jobs/callback_job.h>

typedef struct private_ha_cache_t private_ha_cache_t;

/**
 * Private data of an ha_cache_t object.
 */
struct private_ha_cache_t {

	/**
	 * Public ha_cache_t interface.
	 */
	ha_cache_t public;

	/**
	 * Kernel helper functions
	 */
	ha_kernel_t *kernel;

	/**
	 * Socket to send sync messages over
	 */
	ha_socket_t *socket;

	/**
	 * Tunnel securing sync messages
	 */
	ha_tunnel_t *tunnel;

	/**
	 * Total number of segments
	 */
	u_int count;

	/**
	 * cached entries (ike_sa_t, entry_t)
	 */
	hashtable_t *cache;

	/**
	 * Mutex to lock cache
	 */
	mutex_t *mutex;
};

/**
 * Cache entry for an IKE_SA
 */
typedef struct {
	/* segment this entry is associate to */
	u_int segment;
	/* ADD message */
	ha_message_t *add;
	/* list of updates UPDATE message */
	linked_list_t *updates;
	/* last initiator mid */
	ha_message_t *midi;
	/* last responder mid */
	ha_message_t *midr;
	/* last IV update */
	ha_message_t *iv;
} entry_t;

/**
 * Create a entry with an add message
 */
static entry_t *entry_create(ha_message_t *add)
{
	entry_t *entry;

	INIT(entry,
		.add = add,
		.updates = linked_list_create(),
	);
	return entry;
}

/**
 * clean up a entry
 */
static void entry_destroy(entry_t *entry)
{
	entry->updates->destroy_offset(entry->updates,
									offsetof(ha_message_t, destroy));
	entry->add->destroy(entry->add);
	DESTROY_IF(entry->midi);
	DESTROY_IF(entry->midr);
	DESTROY_IF(entry->iv);
	free(entry);
}

METHOD(ha_cache_t, cache, void,
	private_ha_cache_t *this, ike_sa_t *ike_sa, ha_message_t *message)
{
	entry_t *entry;

	this->mutex->lock(this->mutex);
	switch (message->get_type(message))
	{
		case HA_IKE_ADD:
			entry = entry_create(message);
			entry = this->cache->put(this->cache, ike_sa, entry);
			if (entry)
			{
				entry_destroy(entry);
			}
			break;
		case HA_IKE_UPDATE:
			entry = this->cache->get(this->cache, ike_sa);
			if (entry)
			{
				entry->segment = this->kernel->get_segment(this->kernel,
											ike_sa->get_other_host(ike_sa));
				entry->updates->insert_last(entry->updates, message);
				break;
			}
			message->destroy(message);
			break;
		case HA_IKE_MID_INITIATOR:
			entry = this->cache->get(this->cache, ike_sa);
			if (entry)
			{
				DESTROY_IF(entry->midi);
				entry->midi = message;
				break;
			}
			message->destroy(message);
			break;
		case HA_IKE_MID_RESPONDER:
			entry = this->cache->get(this->cache, ike_sa);
			if (entry)
			{
				DESTROY_IF(entry->midr);
				entry->midr = message;
				break;
			}
			message->destroy(message);
			break;
		case HA_IKE_IV:
			entry = this->cache->get(this->cache, ike_sa);
			if (entry)
			{
				DESTROY_IF(entry->iv);
				entry->iv = message;
				break;
			}
			message->destroy(message);
			break;
		case HA_IKE_DELETE:
			entry = this->cache->remove(this->cache, ike_sa);
			if (entry)
			{
				entry_destroy(entry);
			}
			message->destroy(message);
			break;
		default:
			message->destroy(message);
			break;
	}
	this->mutex->unlock(this->mutex);
}

METHOD(ha_cache_t, delete_, void,
	private_ha_cache_t *this, ike_sa_t *ike_sa)
{
	entry_t *entry;

	this->mutex->lock(this->mutex);
	entry = this->cache->remove(this->cache, ike_sa);
	if (entry)
	{
		entry_destroy(entry);
	}
	this->mutex->unlock(this->mutex);
}

/**
 * Rekey all children of an IKE_SA
 */
static status_t rekey_children(ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	child_sa_t *child_sa;
	status_t status = SUCCESS;
	linked_list_t *children;
	struct {
		protocol_id_t protocol;
		uint32_t spi;
	} *info;

	children = linked_list_create();
	enumerator = ike_sa->create_child_sa_enumerator(ike_sa);
	while (enumerator->enumerate(enumerator, &child_sa))
	{
		INIT(info,
			.protocol = child_sa->get_protocol(child_sa),
			.spi = child_sa->get_spi(child_sa, TRUE),
		);
		children->insert_last(children, info);
	}
	enumerator->destroy(enumerator);

	enumerator = children->create_enumerator(children);
	while (enumerator->enumerate(enumerator, &info))
	{
		if (ike_sa->supports_extension(ike_sa, EXT_MS_WINDOWS) &&
			ike_sa->has_condition(ike_sa, COND_NAT_THERE))
		{
			/* NATed Windows clients don't accept CHILD_SA rekeying, but fail
			 * with an "invalid situation" error. We just close the CHILD_SA,
			 * Windows will reestablish it immediately if required. */
			DBG1(DBG_CFG, "resyncing CHILD_SA using a delete");
			status = ike_sa->delete_child_sa(ike_sa, info->protocol, info->spi,
											 FALSE);
		}
		else
		{
			DBG1(DBG_CFG, "resyncing CHILD_SA using a rekey");
			status = ike_sa->rekey_child_sa(ike_sa, info->protocol, info->spi);
		}
		if (status == DESTROY_ME)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	children->destroy_function(children, free);

	return status;
}

/**
 * Trigger rekeying of CHILD_SA in segment
 */
static void rekey_segment(private_ha_cache_t *this, u_int segment)
{
	ike_sa_t *ike_sa;
	enumerator_t *enumerator;
	linked_list_t *list;
	ike_sa_id_t *id;

	list = linked_list_create();

	enumerator = charon->ike_sa_manager->create_enumerator(
												charon->ike_sa_manager, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (this->tunnel && this->tunnel->is_sa(this->tunnel, ike_sa))
		{
			continue;
		}
		if (ike_sa->get_state(ike_sa) == IKE_ESTABLISHED &&
			this->kernel->get_segment(this->kernel,
						ike_sa->get_other_host(ike_sa)) == segment)
		{
			id = ike_sa->get_id(ike_sa);
			list->insert_last(list, id->clone(id));
		}
	}
	enumerator->destroy(enumerator);

	while (list->remove_last(list, (void**)&id) == SUCCESS)
	{
		ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager, id);
		if (ike_sa)
		{
			if (rekey_children(ike_sa) != DESTROY_ME)
			{
				charon->ike_sa_manager->checkin(
										charon->ike_sa_manager, ike_sa);
			}
			else
			{
				charon->ike_sa_manager->checkin_and_destroy(
										charon->ike_sa_manager, ike_sa);
			}
		}
		id->destroy(id);
	}
	list->destroy(list);
}

METHOD(ha_cache_t, resync, void,
	private_ha_cache_t *this, u_int segment)
{
	enumerator_t *enumerator, *updates;
	ike_sa_t *ike_sa;
	entry_t *entry;
	ha_message_t *message;

	DBG1(DBG_CFG, "resyncing HA segment %d", segment);

	this->mutex->lock(this->mutex);
	enumerator = this->cache->create_enumerator(this->cache);
	while (enumerator->enumerate(enumerator, &ike_sa, &entry))
	{
		if (entry->segment == segment)
		{
			this->socket->push(this->socket, entry->add);
			updates = entry->updates->create_enumerator(entry->updates);
			while (updates->enumerate(updates, &message))
			{
				this->socket->push(this->socket, message);
			}
			updates->destroy(updates);
			if (entry->midi)
			{
				this->socket->push(this->socket, entry->midi);
			}
			if (entry->midr)
			{
				this->socket->push(this->socket, entry->midr);
			}
			if (entry->iv)
			{
				this->socket->push(this->socket, entry->iv);
			}
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);

	rekey_segment(this, segment);
}

/**
 * Request a resync of all segments
 */
static job_requeue_t request_resync(private_ha_cache_t *this)
{
	ha_message_t *message;
	int i;

	DBG1(DBG_CFG, "requesting HA resynchronization");

	message = ha_message_create(HA_RESYNC);
	for (i = 1; i <= this->count; i++)
	{
		message->add_attribute(message, HA_SEGMENT, i);
	}
	this->socket->push(this->socket, message);
	message->destroy(message);
	return JOB_REQUEUE_NONE;
}

METHOD(ha_cache_t, destroy, void,
	private_ha_cache_t *this)
{
	this->cache->destroy(this->cache);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
ha_cache_t *ha_cache_create(ha_kernel_t *kernel, ha_socket_t *socket,
							ha_tunnel_t *tunnel, bool sync, u_int count)
{
	private_ha_cache_t *this;

	INIT(this,
		.public = {
			.cache = _cache,
			.delete = _delete_,
			.resync = _resync,
			.destroy = _destroy,
		},
		.count = count,
		.kernel = kernel,
		.socket = socket,
		.tunnel = tunnel,
		.cache = hashtable_create(hashtable_hash_ptr, hashtable_equals_ptr, 8),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	if (sync)
	{
		/* request a resync as soon as we are up */
		lib->scheduler->schedule_job(lib->scheduler, (job_t*)
			callback_job_create_with_prio((callback_job_cb_t)request_resync,
									this, NULL, NULL, JOB_PRIO_CRITICAL), 1);
	}
	return &this->public;
}
