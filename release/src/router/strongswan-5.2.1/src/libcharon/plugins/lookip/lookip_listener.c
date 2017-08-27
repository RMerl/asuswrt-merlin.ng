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

#include "lookip_listener.h"

#include <daemon.h>
#include <collections/hashtable.h>
#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_lookip_listener_t private_lookip_listener_t;

/**
 * Private data of an lookip_listener_t object.
 */
struct private_lookip_listener_t {

	/**
	 * Public lookip_listener_t interface.
	 */
	lookip_listener_t public;

	/**
	 * Lock for hashtable
	 */
	rwlock_t *lock;

	/**
	 * Hashtable with entries: host_t => entry_t
	 */
	hashtable_t *entries;

	/**
	 * List of registered listeners
	 */
	linked_list_t *listeners;
};

/**
 * Listener entry
 */
typedef struct {
	/** callback function */
	lookip_callback_t cb;
	/** user data for callback */
	void *user;
} listener_entry_t;

/**
 * Hashtable entry
 */
typedef struct {
	/** virtual IP, serves as lookup key */
	host_t *vip;
	/** peers external address */
	host_t *other;
	/** peer (EAP-)Identity */
	identification_t *id;
	/** associated connection name */
	char *name;
	/** IKE_SA unique identifier */
	u_int unique_id;
} entry_t;

/**
 * Destroy a hashtable entry
 */
static void entry_destroy(entry_t *entry)
{
	entry->vip->destroy(entry->vip);
	entry->other->destroy(entry->other);
	entry->id->destroy(entry->id);
	free(entry->name);
	free(entry);
}

/**
 * Hashtable hash function
 */
static u_int hash(host_t *key)
{
	return chunk_hash(key->get_address(key));
}

/**
 * Hashtable equals function
 */
static bool equals(host_t *a, host_t *b)
{
	return a->ip_equals(a, b);
}

/**
 * Compare callback that invokes up callback of all registered listeners
 */
static bool notify_up(listener_entry_t *listener, entry_t *entry)
{
	if (!listener->cb(listener->user, TRUE, entry->vip, entry->other,
					  entry->id, entry->name, entry->unique_id))
	{
		free(listener);
		return TRUE;
	}
	return FALSE;
}

/**
 * Compare callback that invokes down callback of all registered listeners
 */
static bool notify_down(listener_entry_t *listener, entry_t *entry)
{
	if (!listener->cb(listener->user, FALSE, entry->vip, entry->other,
					  entry->id, entry->name, entry->unique_id))
	{
		free(listener);
		return TRUE;
	}
	return FALSE;
}

/**
 * Add a new entry to the hashtable
 */
static void add_entry(private_lookip_listener_t *this, ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	host_t *vip, *other;
	identification_t *id;
	entry_t *entry;

	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, FALSE);
	while (enumerator->enumerate(enumerator, &vip))
	{
		other = ike_sa->get_other_host(ike_sa);
		id = ike_sa->get_other_eap_id(ike_sa);

		INIT(entry,
			.vip = vip->clone(vip),
			.other = other->clone(other),
			.id = id->clone(id),
			.name = strdup(ike_sa->get_name(ike_sa)),
			.unique_id = ike_sa->get_unique_id(ike_sa),
		);

		this->lock->read_lock(this->lock);
		this->listeners->remove(this->listeners, entry, (void*)notify_up);
		this->lock->unlock(this->lock);

		this->lock->write_lock(this->lock);
		entry = this->entries->put(this->entries, entry->vip, entry);
		this->lock->unlock(this->lock);
		if (entry)
		{
			entry_destroy(entry);
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Remove an entry from the hashtable
 */
static void remove_entry(private_lookip_listener_t *this, ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	host_t *vip;
	entry_t *entry;

	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, FALSE);
	while (enumerator->enumerate(enumerator, &vip))
	{
		this->lock->write_lock(this->lock);
		entry = this->entries->remove(this->entries, vip);
		this->lock->unlock(this->lock);
		if (entry)
		{
			this->lock->read_lock(this->lock);
			this->listeners->remove(this->listeners, entry, (void*)notify_down);
			this->lock->unlock(this->lock);

			entry_destroy(entry);
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(listener_t, message_hook, bool,
	private_lookip_listener_t *this, ike_sa_t *ike_sa,
	message_t *message, bool incoming, bool plain)
{
	if (plain && ike_sa->get_state(ike_sa) == IKE_ESTABLISHED &&
		!incoming && !message->get_request(message))
	{
		if (ike_sa->get_version(ike_sa) == IKEV1 &&
			message->get_exchange_type(message) == TRANSACTION)
		{
			add_entry(this, ike_sa);
		}
		if (ike_sa->get_version(ike_sa) == IKEV2 &&
			message->get_exchange_type(message) == IKE_AUTH)
		{
			add_entry(this, ike_sa);
		}
	}
	return TRUE;
}

METHOD(listener_t, ike_updown, bool,
	private_lookip_listener_t *this, ike_sa_t *ike_sa, bool up)
{
	if (!up)
	{
		remove_entry(this, ike_sa);
	}
	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	private_lookip_listener_t *this, ike_sa_t *old, ike_sa_t *new)
{
	/* During IKE_SA rekey, the unique identifier changes. Fire update events
	 * and update the cached entry. During the invocation of this hook, the
	 * virtual IPs have been migrated to new, hence remove that entry. */
	remove_entry(this, new);
	add_entry(this, new);

	return TRUE;
}

METHOD(lookip_listener_t, lookup, int,
	private_lookip_listener_t *this, host_t *vip,
	lookip_callback_t cb, void *user)
{
	entry_t *entry;
	int matches = 0;

	this->lock->read_lock(this->lock);
	if (vip)
	{
		entry = this->entries->get(this->entries, vip);
		if (entry)
		{
			cb(user, TRUE, entry->vip, entry->other, entry->id,
			   entry->name, entry->unique_id);
			matches ++;
		}
	}
	else
	{
		enumerator_t *enumerator;

		enumerator = this->entries->create_enumerator(this->entries);
		while (enumerator->enumerate(enumerator, &vip, &entry))
		{
			cb(user, TRUE, entry->vip, entry->other, entry->id,
			   entry->name, entry->unique_id);
			matches++;
		}
		enumerator->destroy(enumerator);
	}
	this->lock->unlock(this->lock);

	return matches;
}

METHOD(lookip_listener_t, add_listener, void,
	private_lookip_listener_t *this, lookip_callback_t cb, void *user)
{
	listener_entry_t *listener;

	INIT(listener,
		.cb = cb,
		.user = user,
	);

	this->lock->write_lock(this->lock);
	this->listeners->insert_last(this->listeners, listener);
	this->lock->unlock(this->lock);
}

METHOD(lookip_listener_t, remove_listener, void,
	private_lookip_listener_t *this, void *user)
{
	listener_entry_t *listener;
	enumerator_t *enumerator;

	this->lock->write_lock(this->lock);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &listener))
	{
		if (listener->user == user)
		{
			this->listeners->remove_at(this->listeners, enumerator);
			free(listener);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(lookip_listener_t, destroy, void,
	private_lookip_listener_t *this)
{
	this->listeners->destroy_function(this->listeners, free);
	this->entries->destroy(this->entries);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
lookip_listener_t *lookip_listener_create()
{
	private_lookip_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.message = _message_hook,
				.ike_updown = _ike_updown,
				.ike_rekey = _ike_rekey,
			},
			.lookup = _lookup,
			.add_listener = _add_listener,
			.remove_listener = _remove_listener,
			.destroy = _destroy,
		},
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.entries = hashtable_create((hashtable_hash_t)hash,
									(hashtable_equals_t)equals, 32),
		.listeners = linked_list_create(),
	);

	return &this->public;
}
