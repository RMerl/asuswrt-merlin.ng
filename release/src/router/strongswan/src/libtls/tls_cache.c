/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "tls_cache.h"

#include <utils/debug.h>
#include <collections/linked_list.h>
#include <collections/hashtable.h>
#include <threading/rwlock.h>

typedef struct private_tls_cache_t private_tls_cache_t;

/**
 * Private data of an tls_cache_t object.
 */
struct private_tls_cache_t {

	/**
	 * Public tls_cache_t interface.
	 */
	tls_cache_t public;

	/**
	 * Mapping session => entry_t, fast lookup by session
	 */
	hashtable_t *table;

	/**
	 * List containing all entries
	 */
	linked_list_t *list;

	/**
	 * Lock to list and table
	 */
	rwlock_t *lock;

	/**
	 * Session limit
	 */
	u_int max_sessions;

	/**
	 * maximum age of a session, in seconds
	 */
	u_int max_age;
};

/**
 * Hashtable entry
 */
typedef struct {
	/** session identifier */
	chunk_t session;
	/** master secret */
	chunk_t master;
	/** TLS cipher suite */
	tls_cipher_suite_t suite;
	/** optional identity this entry is bound to */
	identification_t *id;
	/** time of add */
	time_t t;
} entry_t;

/**
 * Destroy an entry
 */
static void entry_destroy(entry_t *entry)
{
	chunk_clear(&entry->session);
	chunk_clear(&entry->master);
	DESTROY_IF(entry->id);
	free(entry);
}

/**
 * Hashtable hash function
 */
static u_int hash(chunk_t *key)
{
	return chunk_hash(*key);
}

/**
 * Hashtable equals function
 */
static bool equals(chunk_t *a, chunk_t *b)
{
	return chunk_equals(*a, *b);
}

METHOD(tls_cache_t, create_, void,
	private_tls_cache_t *this, chunk_t session, identification_t *id,
	chunk_t master, tls_cipher_suite_t suite)
{
	entry_t *entry;

	INIT(entry,
		.session = chunk_clone(session),
		.master = chunk_clone(master),
		.suite = suite,
		.id = id ? id->clone(id) : NULL,
		.t = time_monotonic(NULL),
	);

	this->lock->write_lock(this->lock);
	this->list->insert_first(this->list, entry);
	this->table->put(this->table, &entry->session, entry);
	if (this->list->get_count(this->list) > this->max_sessions &&
		this->list->remove_last(this->list, (void**)&entry) == SUCCESS)
	{
		DBG2(DBG_TLS, "session limit of %u reached, deleting %#B",
			 this->max_sessions, &entry->session);
		this->table->remove(this->table, &entry->session);
		entry_destroy(entry);
	}
	this->lock->unlock(this->lock);

	DBG2(DBG_TLS, "created TLS session %#B, %d sessions",
		 &session, this->list->get_count(this->list));
}

METHOD(tls_cache_t, lookup, tls_cipher_suite_t,
	private_tls_cache_t *this, chunk_t session, identification_t *id,
	chunk_t* master)
{
	tls_cipher_suite_t suite = 0;
	entry_t *entry;
	time_t now;
	u_int age;

	now = time_monotonic(NULL);

	this->lock->write_lock(this->lock);
	entry = this->table->get(this->table, &session);
	if (entry)
	{
		age = now - entry->t;
		if (age <= this->max_age)
		{
			if (!id || !entry->id || id->equals(id, entry->id))
			{
				*master = chunk_clone(entry->master);
				suite = entry->suite;
			}
		}
		else
		{
			DBG2(DBG_TLS, "TLS session %#B expired: %u seconds", &session, age);
		}
	}
	this->lock->unlock(this->lock);

	if (suite)
	{
		DBG2(DBG_TLS, "resuming TLS session %#B, age %u seconds", &session, age);
	}
	return suite;
}

METHOD(tls_cache_t, check, chunk_t,
	private_tls_cache_t *this, identification_t *id)
{
	chunk_t session = chunk_empty;
	enumerator_t *enumerator;
	entry_t *entry;
	time_t now;

	now = time_monotonic(NULL);
	this->lock->read_lock(this->lock);
	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->t + this->max_age >= now &&
			entry->id && id->equals(id, entry->id))
		{
			session = chunk_clone(entry->session);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return session;
}

METHOD(tls_cache_t, destroy, void,
	private_tls_cache_t *this)
{
	entry_t *entry;

	while (this->list->remove_last(this->list, (void**)&entry) == SUCCESS)
	{
		entry_destroy(entry);
	}
	this->list->destroy(this->list);
	this->table->destroy(this->table);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
tls_cache_t *tls_cache_create(u_int max_sessions, u_int max_age)
{
	private_tls_cache_t *this;

	INIT(this,
		.public = {
			.create = _create_,
			.lookup = _lookup,
			.check = _check,
			.destroy = _destroy,
		},
		.table = hashtable_create((hashtable_hash_t)hash,
								  (hashtable_equals_t)equals, 8),
		.list = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.max_sessions = max_sessions,
		.max_age = max_age,
	);

	return &this->public;
}
