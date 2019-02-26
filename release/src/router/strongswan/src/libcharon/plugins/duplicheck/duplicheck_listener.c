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

#include "duplicheck_listener.h"

#include <daemon.h>
#include <threading/mutex.h>
#include <collections/hashtable.h>
#include <encoding/payloads/delete_payload.h>
#include <processing/jobs/delete_ike_sa_job.h>

typedef struct private_duplicheck_listener_t private_duplicheck_listener_t;

/**
 * Private data of an duplicheck_listener_t object.
 */
struct private_duplicheck_listener_t {

	/**
	 * Public duplicheck_listener_t interface.
	 */
	duplicheck_listener_t public;

	/**
	 * Socket to send notifications to
	 */
	duplicheck_notify_t *notify;

	/**
	 * Mutex to lock hashtables
	 */
	mutex_t *mutex;

	/**
	 * Hashtable of active IKE_SAs, identification_t => entry_t
	 */
	hashtable_t *active;

	/**
	 * Hashtable with active liveness checks, identification_t => entry_t
	 */
	hashtable_t *checking;
};

/**
 * Entry for hashtables
 */
typedef struct {
	/** peer identity */
	identification_t *id;
	/** list of IKE_SA identifiers, ike_sa_id_t */
	linked_list_t *sas;
} entry_t;

/**
 * Destroy a hashtable entry
 */
static void entry_destroy(entry_t *this)
{
	this->id->destroy(this->id);
	this->sas->destroy_offset(this->sas, offsetof(ike_sa_id_t, destroy));
	free(this);
}

/**
 * Hashtable hash function
 */
static u_int hash(identification_t *key)
{
	return chunk_hash(key->get_encoding(key));
}

/**
 * Hashtable equals function
 */
static bool equals(identification_t *a, identification_t *b)
{
	return a->equals(a, b);
}

/**
 * Put an IKE_SA identifier to hashtable
 */
static void put(hashtable_t *table, identification_t *id, ike_sa_id_t *sa)
{
	entry_t *entry;

	entry = table->get(table, id);
	if (!entry)
	{
		INIT(entry,
			.id = id->clone(id),
			.sas = linked_list_create(),
		);
		table->put(table, entry->id, entry);
	}
	entry->sas->insert_last(entry->sas, sa->clone(sa));
}

/**
 * Purge an entry from table if it has no IKE_SA identifiers
 */
static void remove_if_empty(hashtable_t *table, entry_t *entry)
{
	if (entry->sas->get_count(entry->sas) == 0)
	{
		entry = table->remove(table, entry->id);
		if (entry)
		{
			entry_destroy(entry);
		}
	}
}

/**
 * Remove the first entry found in the table for the given id
 */
static ike_sa_id_t *remove_first(hashtable_t *table, identification_t *id)
{
	ike_sa_id_t *sa = NULL;
	entry_t *entry;

	entry = table->get(table, id);
	if (entry)
	{
		entry->sas->remove_first(entry->sas, (void**)&sa);
		remove_if_empty(table, entry);
	}
	return sa;
}

/**
 * Remove a specific IKE_SA ID for the given identity
 */
static bool remove_specific(hashtable_t *table, identification_t *id,
							ike_sa_id_t *sa)
{
	enumerator_t *enumerator;
	bool found = FALSE;
	entry_t *entry;
	ike_sa_id_t *current;

	entry = table->get(table, id);
	if (entry)
	{
		enumerator = entry->sas->create_enumerator(entry->sas);
		while (enumerator->enumerate(enumerator, &current))
		{
			if (sa->equals(sa, current))
			{
				entry->sas->remove_at(entry->sas, enumerator);
				current->destroy(current);
				found = TRUE;
				break;
			}
		}
		enumerator->destroy(enumerator);
		if (found)
		{
			remove_if_empty(table, entry);
		}
	}
	return found;
}

METHOD(listener_t, ike_rekey, bool,
	private_duplicheck_listener_t *this, ike_sa_t *old, ike_sa_t *new)
{
	this->mutex->lock(this->mutex);

	remove_specific(this->active, old->get_other_id(old), old->get_id(old));
	put(this->active, new->get_other_id(new), new->get_id(new));

	this->mutex->unlock(this->mutex);

	return TRUE;
}

METHOD(listener_t, ike_updown, bool,
	private_duplicheck_listener_t *this, ike_sa_t *ike_sa, bool up)
{
	identification_t *id;
	ike_sa_id_t *sa;

	id = ike_sa->get_other_id(ike_sa);

	this->mutex->lock(this->mutex);
	if (up)
	{
		/* another IKE_SA for this identity active? */
		sa = remove_first(this->active, id);
		if (sa)
		{
			DBG1(DBG_CFG, "detected duplicate IKE_SA for '%Y', "
				 "triggering delete for old IKE_SA", id);
			put(this->checking, id, sa);
			lib->processor->queue_job(lib->processor,
								(job_t*)delete_ike_sa_job_create(sa, TRUE));
			sa->destroy(sa);
		}
		/* register IKE_SA as the new active */
		sa = ike_sa->get_id(ike_sa);
		put(this->active, id, sa);
	}
	else
	{
		sa = ike_sa->get_id(ike_sa);
		/* check if closing an IKE_SA currently in checking state */
		if (remove_specific(this->checking, id, sa))
		{
			DBG1(DBG_CFG, "delete for duplicate IKE_SA '%Y' timed out, "
				 "keeping new IKE_SA", id);
		}
		/* check normal close of IKE_SA */
		remove_specific(this->active, id, sa);
	}
	this->mutex->unlock(this->mutex);

	return TRUE;
}

METHOD(listener_t, message_hook, bool,
	private_duplicheck_listener_t *this, ike_sa_t *ike_sa,
	message_t *message, bool incoming, bool plain)
{
	if (incoming && plain && !message->get_request(message))
	{
		identification_t *id;
		ike_sa_id_t *sa;

		id = ike_sa->get_other_id(ike_sa);
		sa = ike_sa->get_id(ike_sa);

		this->mutex->lock(this->mutex);
		if (remove_specific(this->checking, id, sa))
		{
			DBG1(DBG_CFG, "got a response on a duplicate IKE_SA for '%Y', "
				 "deleting new IKE_SA", id);
			charon->bus->alert(charon->bus, ALERT_UNIQUE_KEEP);
			sa = remove_first(this->active, id);
			if (sa)
			{
				lib->processor->queue_job(lib->processor,
								(job_t*)delete_ike_sa_job_create(sa, TRUE));
				sa->destroy(sa);
			}
			this->mutex->unlock(this->mutex);

			this->notify->send(this->notify, id);
		}
		else
		{
			this->mutex->unlock(this->mutex);
		}
	}
	return TRUE;
}

METHOD(duplicheck_listener_t, destroy, void,
	private_duplicheck_listener_t *this)
{
	enumerator_t *enumerator;
	identification_t *key;
	entry_t *value;

	enumerator = this->active->create_enumerator(this->active);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		entry_destroy(value);
	}
	enumerator->destroy(enumerator);

	enumerator = this->checking->create_enumerator(this->checking);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		entry_destroy(value);
	}
	enumerator->destroy(enumerator);

	this->active->destroy(this->active);
	this->checking->destroy(this->checking);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
duplicheck_listener_t *duplicheck_listener_create(duplicheck_notify_t *notify)
{
	private_duplicheck_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.ike_rekey = _ike_rekey,
				.ike_updown = _ike_updown,
				.message = _message_hook,
			},
			.destroy = _destroy,
		},
		.notify = notify,
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.active = hashtable_create((hashtable_hash_t)hash,
								   (hashtable_equals_t)equals, 32),
		.checking = hashtable_create((hashtable_hash_t)hash,
									 (hashtable_equals_t)equals, 2),
	);

	return &this->public;
}
