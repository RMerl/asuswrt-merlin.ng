/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "eap_radius_provider.h"

#include <daemon.h>
#include <collections/hashtable.h>
#include <threading/mutex.h>

typedef struct private_eap_radius_provider_t private_eap_radius_provider_t;
typedef struct private_listener_t private_listener_t;

/**
 * Private data of registered listener
 */
struct private_listener_t {

	/**
	 * Implements listener_t interface
	 */
	listener_t public;

	/**
	 * Leases not acquired yet, identification_t => entry_t
	 */
	hashtable_t *unclaimed;

	/**
	 * Leases acquired, identification_t => entry_t
	 */
	hashtable_t *claimed;

	/**
	 * Mutex to lock leases
	 */
	mutex_t *mutex;
};

/**
 * Private data of an eap_radius_provider_t object.
 */
struct private_eap_radius_provider_t {

	/**
	 * Public eap_radius_provider_t interface.
	 */
	eap_radius_provider_t public;

	/**
	 * Additionally implements the listener_t interface
	 */
	private_listener_t listener;
};

/**
 * Singleton instance of provider
 */
static eap_radius_provider_t *singleton = NULL;

/**
 * Configuration attribute in an entry
 */
typedef struct {
	/** type of attribute */
	configuration_attribute_type_t type;
	/** attribute data */
	chunk_t data;
} attr_t;

/**
 * Destroy an attr_t
 */
static void destroy_attr(attr_t *this)
{
	free(this->data.ptr);
	free(this);
}

/**
 * Hashtable entry with leases and attributes
 */
typedef struct {
	/** IKE_SA unique id we assign the IP lease */
	uintptr_t id;
	/** list of IP leases received from AAA, as host_t */
	linked_list_t *addrs;
	/** list of configuration attributes, as attr_t */
	linked_list_t *attrs;
} entry_t;

/**
 * destroy an entry_t
 */
static void destroy_entry(entry_t *this)
{
	this->addrs->destroy_offset(this->addrs, offsetof(host_t, destroy));
	this->attrs->destroy_function(this->attrs, (void*)destroy_attr);
	free(this);
}

/**
 * Get or create an entry from a locked hashtable
 */
static entry_t* get_or_create_entry(hashtable_t *hashtable, uintptr_t id)
{
	entry_t *entry;

	entry = hashtable->get(hashtable, (void*)id);
	if (!entry)
	{
		INIT(entry,
			.id = id,
			.addrs = linked_list_create(),
			.attrs = linked_list_create(),
		);
		hashtable->put(hashtable, (void*)id, entry);
	}
	return entry;
}

/**
 * Put an entry to hashtable, or destroy it if empty
 */
static void put_or_destroy_entry(hashtable_t *hashtable, entry_t *entry)
{
	if (entry->addrs->get_count(entry->addrs) > 0 ||
		entry->attrs->get_count(entry->attrs) > 0)
	{
		hashtable->put(hashtable, (void*)entry->id, entry);
	}
	else
	{
		destroy_entry(entry);
	}
}

/**
 * Hashtable hash function
 */
static u_int hash(uintptr_t id)
{
	return id;
}

/**
 * Hashtable equals function
 */
static bool equals(uintptr_t a, uintptr_t b)
{
	return a == b;
}

/**
 * Insert an address entry to a locked claimed/unclaimed hashtable
 */
static void add_addr(private_eap_radius_provider_t *this,
					 hashtable_t *hashtable, uintptr_t id, host_t *host)
{
	entry_t *entry;

	entry = get_or_create_entry(hashtable, id);
	entry->addrs->insert_last(entry->addrs, host);
}

/**
 * Remove the next address from the locked hashtable stored for given id
 */
static host_t* remove_addr(private_eap_radius_provider_t *this,
						   hashtable_t *hashtable, uintptr_t id, host_t *addr)
{
	enumerator_t *enumerator;
	entry_t *entry;
	host_t *found = NULL, *current;

	entry = hashtable->remove(hashtable, (void*)id);
	if (entry)
	{
		enumerator = entry->addrs->create_enumerator(entry->addrs);
		while (enumerator->enumerate(enumerator, &current))
		{
			if (addr->ip_equals(addr, current))
			{	/* prefer an exact match */
				entry->addrs->remove_at(entry->addrs, enumerator);
				enumerator->destroy(enumerator);
				put_or_destroy_entry(hashtable, entry);
				return current;
			}
			if (!found && addr->get_family(addr) == current->get_family(current))
			{	/* fallback to the first IP with a matching address family */
				found = current;
			}
		}
		enumerator->destroy(enumerator);
		if (found)
		{
			entry->addrs->remove(entry->addrs, found, NULL);
		}
		put_or_destroy_entry(hashtable, entry);
	}
	return found;
}

/**
 * Insert an attribute entry to a locked claimed/unclaimed hashtable
 */
static void add_attr(private_eap_radius_provider_t *this,
					 hashtable_t *hashtable, uintptr_t id, attr_t *attr)
{
	entry_t *entry;

	entry = get_or_create_entry(hashtable, id);
	entry->attrs->insert_last(entry->attrs, attr);
}

/**
 * Remove the next attribute from the locked hashtable stored for given id
 */
static attr_t* remove_attr(private_eap_radius_provider_t *this,
						   hashtable_t *hashtable, uintptr_t id)
{
	entry_t *entry;
	attr_t *attr = NULL;

	entry = hashtable->remove(hashtable, (void*)id);
	if (entry)
	{
		entry->attrs->remove_first(entry->attrs, (void**)&attr);
		put_or_destroy_entry(hashtable, entry);
	}
	return attr;
}

/**
 * Clean up unclaimed leases assigned for an IKE_SA
 */
static void release_unclaimed(private_listener_t *this, ike_sa_t *ike_sa)
{
	uintptr_t id;
	entry_t *entry;

	id = ike_sa->get_unique_id(ike_sa);
	this->mutex->lock(this->mutex);
	entry = this->unclaimed->remove(this->unclaimed, (void*)id);
	this->mutex->unlock(this->mutex);
	if (entry)
	{
		destroy_entry(entry);
	}
}

METHOD(listener_t, message_hook, bool,
	private_listener_t *this, ike_sa_t *ike_sa,
	message_t *message, bool incoming, bool plain)
{
	if (plain && ike_sa->get_state(ike_sa) == IKE_ESTABLISHED &&
		!incoming && !message->get_request(message))
	{
		if ((ike_sa->get_version(ike_sa) == IKEV1 &&
			 message->get_exchange_type(message) == TRANSACTION) ||
			(ike_sa->get_version(ike_sa) == IKEV2 &&
			 message->get_exchange_type(message) == IKE_AUTH))
		{
			/* if the addresses have not been claimed yet, they won't. Release
			 * these resources. */
			release_unclaimed(this, ike_sa);
		}
	}
	return TRUE;
}

METHOD(listener_t, ike_updown, bool,
	private_listener_t *this, ike_sa_t *ike_sa, bool up)
{
	if (!up)
	{
		/* if the message hook does not apply because of a failed exchange
		 * or something, make sure we release any resources now */
		release_unclaimed(this, ike_sa);
	}
	return TRUE;
}

/**
 * Migrate an entry in hashtable from old to new id
 */
static void migrate_entry(hashtable_t *table, uintptr_t old, uintptr_t new)
{
	entry_t *entry;

	entry = table->remove(table, (void*)old);
	if (entry)
	{
		entry->id = new;
		entry = table->put(table, (void*)new, entry);
		if (entry)
		{	/* shouldn't happen */
			destroy_entry(entry);
		}
	}
}

METHOD(listener_t, ike_rekey, bool,
	private_listener_t *this, ike_sa_t *old, ike_sa_t *new)
{
	uintptr_t old_id, new_id;

	old_id = old->get_unique_id(old);
	new_id = new->get_unique_id(new);

	this->mutex->lock(this->mutex);

	migrate_entry(this->unclaimed, old_id, new_id);
	migrate_entry(this->claimed, old_id, new_id);

	this->mutex->unlock(this->mutex);

	return TRUE;
}

METHOD(attribute_provider_t, acquire_address, host_t*,
	private_eap_radius_provider_t *this, linked_list_t *pools,
	ike_sa_t *ike_sa, host_t *requested)
{
	enumerator_t *enumerator;
	host_t *addr = NULL;
	uintptr_t sa;
	char *name;

	sa = ike_sa->get_unique_id(ike_sa);

	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &name))
	{
		if (streq(name, "radius"))
		{
			this->listener.mutex->lock(this->listener.mutex);
			addr = remove_addr(this, this->listener.unclaimed, sa, requested);
			if (addr)
			{
				add_addr(this, this->listener.claimed, sa, addr->clone(addr));
			}
			this->listener.mutex->unlock(this->listener.mutex);
			break;
		}
	}
	enumerator->destroy(enumerator);

	return addr;
}

METHOD(attribute_provider_t, release_address, bool,
	private_eap_radius_provider_t *this, linked_list_t *pools, host_t *address,
	ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	host_t *found = NULL;
	uintptr_t sa;
	char *name;

	sa = ike_sa->get_unique_id(ike_sa);

	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &name))
	{
		if (streq(name, "radius"))
		{
			this->listener.mutex->lock(this->listener.mutex);
			found = remove_addr(this, this->listener.claimed, sa, address);
			this->listener.mutex->unlock(this->listener.mutex);
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (found)
	{
		found->destroy(found);
		return TRUE;
	}
	return FALSE;
}

/**
 * Enumerator implementation over attributes
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** list of attributes to enumerate */
	linked_list_t *list;
	/** currently enumerating attribute */
	attr_t *current;
} attribute_enumerator_t;

METHOD(enumerator_t, attribute_enumerate, bool,
	attribute_enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	if (this->current)
	{
		destroy_attr(this->current);
		this->current = NULL;
	}
	if (this->list->remove_first(this->list, (void**)&this->current) == SUCCESS)
	{
		*type = this->current->type;
		*data = this->current->data;
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, attribute_destroy, void,
	attribute_enumerator_t *this)
{
	if (this->current)
	{
		destroy_attr(this->current);
	}
	this->list->destroy_function(this->list, (void*)destroy_attr);
	free(this);
}

METHOD(attribute_provider_t, create_attribute_enumerator, enumerator_t*,
	private_eap_radius_provider_t *this, linked_list_t *pools,
	ike_sa_t *ike_sa, linked_list_t *vips)
{
	attribute_enumerator_t *enumerator;
	attr_t *attr;
	uintptr_t sa;

	sa = ike_sa->get_unique_id(ike_sa);

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _attribute_enumerate,
			.destroy = _attribute_destroy,
		},
		.list = linked_list_create(),
	);

	/* we forward attributes regardless of pool configurations */
	this->listener.mutex->lock(this->listener.mutex);
	while (TRUE)
	{
		attr = remove_attr(this, this->listener.unclaimed, sa);
		if (!attr)
		{
			break;
		}
		enumerator->list->insert_last(enumerator->list, attr);
	}
	this->listener.mutex->unlock(this->listener.mutex);

	return &enumerator->public;
}

METHOD(eap_radius_provider_t, add_framed_ip, void,
	private_eap_radius_provider_t *this, uint32_t id, host_t *ip)
{
	this->listener.mutex->lock(this->listener.mutex);
	add_addr(this, this->listener.unclaimed, id, ip);
	this->listener.mutex->unlock(this->listener.mutex);
}

METHOD(eap_radius_provider_t, add_attribute, void,
	private_eap_radius_provider_t *this, uint32_t id,
	configuration_attribute_type_t type, chunk_t data)
{
	attr_t *attr;

	INIT(attr,
		.type = type,
		.data = chunk_clone(data),
	);
	this->listener.mutex->lock(this->listener.mutex);
	add_attr(this, this->listener.unclaimed, id, attr);
	this->listener.mutex->unlock(this->listener.mutex);
}

METHOD(eap_radius_provider_t, clear_unclaimed, enumerator_t*,
	private_eap_radius_provider_t *this, uint32_t id)
{
	entry_t *entry;

	this->listener.mutex->lock(this->listener.mutex);
	entry = this->listener.unclaimed->remove(this->listener.unclaimed,
											 (void*)(uintptr_t)id);
	this->listener.mutex->unlock(this->listener.mutex);
	if (!entry)
	{
		return enumerator_create_empty();
	}
	return enumerator_create_cleaner(
					entry->addrs->create_enumerator(entry->addrs),
					(void*)destroy_entry, entry);
}

METHOD(eap_radius_provider_t, destroy, void,
	private_eap_radius_provider_t *this)
{
	singleton = NULL;
	charon->bus->remove_listener(charon->bus, &this->listener.public);
	this->listener.mutex->destroy(this->listener.mutex);
	this->listener.claimed->destroy(this->listener.claimed);
	this->listener.unclaimed->destroy(this->listener.unclaimed);
	free(this);
}

/**
 * See header
 */
eap_radius_provider_t *eap_radius_provider_create()
{
	if (!singleton)
	{
		private_eap_radius_provider_t *this;

		INIT(this,
			.public = {
				.provider = {
					.acquire_address = _acquire_address,
					.release_address = _release_address,
					.create_attribute_enumerator = _create_attribute_enumerator,
				},
				.add_framed_ip = _add_framed_ip,
				.add_attribute = _add_attribute,
				.clear_unclaimed = _clear_unclaimed,
				.destroy = _destroy,
			},
			.listener = {
				.public = {
					.ike_updown = _ike_updown,
					.ike_rekey = _ike_rekey,
					.message = _message_hook,
				},
				.claimed = hashtable_create((hashtable_hash_t)hash,
										(hashtable_equals_t)equals, 32),
				.unclaimed = hashtable_create((hashtable_hash_t)hash,
										(hashtable_equals_t)equals, 32),
				.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
			},
		);

		if (lib->settings->get_bool(lib->settings,
							"%s.plugins.eap-radius.accounting", FALSE, lib->ns))
		{
			/* if RADIUS accounting is enabled, keep unclaimed IPs around until
			 * the Accounting-Stop message is sent */
			this->listener.public.message = NULL;
		}

		charon->bus->add_listener(charon->bus, &this->listener.public);

		singleton = &this->public;
	}
	return singleton;
}

/**
 * See header
 */
eap_radius_provider_t *eap_radius_provider_get()
{
	return singleton;
}
