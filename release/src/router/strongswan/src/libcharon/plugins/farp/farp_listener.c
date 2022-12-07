/*
 * Copyright (C) 2019 Tobias Brunner
 * Copyright (C) 2010 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include "farp_listener.h"

#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_farp_listener_t private_farp_listener_t;

/**
 * Private data of an farp_listener_t object.
 */
struct private_farp_listener_t {

	/**
	 * Public farp_listener_t interface.
	 */
	farp_listener_t public;

	/**
	 * List with entry_t
	 */
	linked_list_t *entries;

	/**
	 * RWlock for IP list
	 */
	rwlock_t *lock;
};

/**
 * Traffic selector cache entry
 */
typedef struct {
	/** list of local selectors */
	linked_list_t *local;
	/** list of remote selectors */
	linked_list_t *remote;
	/** reqid of CHILD_SA */
	uint32_t reqid;
} entry_t;

/**
 * Destroy a cache entry
 */
static void destroy_entry(entry_t *this)
{
	this->local->destroy_offset(this->local,
								offsetof(traffic_selector_t, destroy));
	this->remote->destroy_offset(this->remote,
								 offsetof(traffic_selector_t, destroy));
	free(this);
}

METHOD(listener_t, child_updown, bool,
	private_farp_listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	bool up)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	entry_t *entry;
	const chunk_t full_from = chunk_from_chars(0x00, 0x00, 0x00, 0x00),
				  full_to   = chunk_from_chars(0xff, 0xff, 0xff, 0xff);

	if (up)
	{
		INIT(entry,
			.local = linked_list_create(),
			.remote = linked_list_create(),
			.reqid = child_sa->get_reqid(child_sa),
		);

		enumerator = child_sa->create_ts_enumerator(child_sa, FALSE);
		while (enumerator->enumerate(enumerator, &ts))
		{
			if (ts->get_type(ts) != TS_IPV4_ADDR_RANGE)
			{
				continue;
			}
			/* ignore 0.0.0.0/0 remote TS because we don't want to
			 * reply to ARP requests for locally connected subnets */
			if (chunk_equals(ts->get_from_address(ts), full_from) &&
				chunk_equals(ts->get_to_address(ts), full_to))
			{
				continue;
			}
			entry->remote->insert_last(entry->remote, ts->clone(ts));
		}
		enumerator->destroy(enumerator);

		enumerator = child_sa->create_ts_enumerator(child_sa, TRUE);
		while (enumerator->enumerate(enumerator, &ts))
		{
			if (ts->get_type(ts) != TS_IPV4_ADDR_RANGE)
			{
				continue;
			}
			entry->local->insert_last(entry->local, ts->clone(ts));
		}
		enumerator->destroy(enumerator);

		if (!entry->remote->get_count(entry->remote) ||
			!entry->local->get_count(entry->local))
		{
			destroy_entry(entry);
			return TRUE;
		}

		this->lock->write_lock(this->lock);
		this->entries->insert_last(this->entries, entry);
		this->lock->unlock(this->lock);
	}
	else
	{
		this->lock->write_lock(this->lock);
		enumerator = this->entries->create_enumerator(this->entries);
		while (enumerator->enumerate(enumerator, &entry))
		{
			if (entry->reqid == child_sa->get_reqid(child_sa))
			{
				this->entries->remove_at(this->entries, enumerator);
				destroy_entry(entry);
				break;
			}
		}
		enumerator->destroy(enumerator);
		this->lock->unlock(this->lock);
	}
	return TRUE;
}

METHOD(farp_listener_t, has_tunnel, bool,
	private_farp_listener_t *this, host_t *local, host_t *remote)
{
	enumerator_t *entries, *locals, *remotes;
	traffic_selector_t *ts;
	bool found = FALSE;
	entry_t *entry;

	this->lock->read_lock(this->lock);
	entries = this->entries->create_enumerator(this->entries);
	while (!found && entries->enumerate(entries, &entry))
	{
		remotes = entry->remote->create_enumerator(entry->remote);
		while (!found && remotes->enumerate(remotes, &ts))
		{
			if (ts->includes(ts, remote))
			{
				locals = entry->local->create_enumerator(entry->local);
				while (!found && locals->enumerate(locals, &ts))
				{
					found = ts->includes(ts, local);
				}
				locals->destroy(locals);
			}
		}
		remotes->destroy(remotes);
	}
	entries->destroy(entries);
	this->lock->unlock(this->lock);

	return found;
}

METHOD(farp_listener_t, destroy, void,
	private_farp_listener_t *this)
{
	this->entries->destroy(this->entries);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
farp_listener_t *farp_listener_create()
{
	private_farp_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.child_updown = _child_updown,
			},
			.has_tunnel = _has_tunnel,
			.destroy = _destroy,
		},
		.entries = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
