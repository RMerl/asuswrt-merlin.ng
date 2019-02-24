/*
 * Copyright (C) 2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include "counters_listener.h"
#include "counters_query.h"

#include <threading/spinlock.h>
#include <collections/hashtable.h>
#include <collections/array.h>

typedef struct private_counters_listener_t private_counters_listener_t;
typedef struct private_counters_query_t private_counters_query_t;

/**
 * Query interface
 */
struct private_counters_query_t {

	/**
	 * Public interface
	 */
	counters_query_t public;

	/**
	 * Reference to this
	 */
	private_counters_listener_t *this;
};

/**
 * Private data
 */
struct private_counters_listener_t {

	/**
	 * Public interface
	 */
	counters_listener_t public;

	/**
	 * Query interface
	 */
	private_counters_query_t query;

	/**
	 * Global counter values
	 */
	uint64_t counters[COUNTER_MAX];

	/**
	 * Counters for specific connection names, char* => entry_t
	 */
	hashtable_t *conns;

	/**
	 * Lock for counter values
	 */
	spinlock_t *lock;
};

/**
 * Counters for a specific connection name
 */
typedef struct {
	/** connection name */
	char *name;
	/** counter values for connection */
	uint64_t counters[COUNTER_MAX];
} entry_t;

/**
 * Destroy named entry
 */
static void destroy_entry(entry_t *this)
{
	free(this->name);
	free(this);
}

/**
 * Hashtable hash function
 */
static u_int hash(char *name)
{
	return chunk_hash(chunk_from_str(name));
}

/**
 * Hashtable equals function
 */
static bool equals(char *a, char *b)
{
	return streq(a, b);
}

/**
 * Get the name of an IKE_SA, but return NULL if it is not known yet
 */
static char *get_ike_sa_name(ike_sa_t *ike_sa)
{
	peer_cfg_t *peer_cfg;

	if (ike_sa)
	{
		peer_cfg = ike_sa->get_peer_cfg(ike_sa);
		if (peer_cfg)
		{
			return peer_cfg->get_name(peer_cfg);
		}
	}
	return NULL;
}

/**
 * Increase a counter for a named entry
 */
static void count_named(private_counters_listener_t *this,
						ike_sa_t *ike_sa, counter_type_t type)
{
	entry_t *entry;
	char *name;

	name = get_ike_sa_name(ike_sa);
	if (name)
	{
		entry = this->conns->get(this->conns, name);
		if (!entry)
		{
			INIT(entry,
				.name = strdup(name),
			);
			this->conns->put(this->conns, entry->name, entry);
		}
		entry->counters[type]++;
	}
}

METHOD(listener_t, alert, bool,
	private_counters_listener_t *this, ike_sa_t *ike_sa,
	alert_t alert, va_list args)
{
	counter_type_t type;

	switch (alert)
	{
		case ALERT_INVALID_IKE_SPI:
			type = COUNTER_IN_INVALID_IKE_SPI;
			break;
		case ALERT_PARSE_ERROR_HEADER:
		case ALERT_PARSE_ERROR_BODY:
			type = COUNTER_IN_INVALID;
			break;
		default:
			return TRUE;
	}

	this->lock->lock(this->lock);
	this->counters[type]++;
	count_named(this, ike_sa, type);
	this->lock->unlock(this->lock);

	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	private_counters_listener_t *this, ike_sa_t *old, ike_sa_t *new)
{
	counter_type_t type;
	ike_sa_id_t *id;

	id = new->get_id(new);
	if (id->is_initiator(id))
	{
		type = COUNTER_INIT_IKE_SA_REKEY;
	}
	else
	{
		type = COUNTER_RESP_IKE_SA_REKEY;
	}

	this->lock->lock(this->lock);
	this->counters[type]++;
	count_named(this, old, type);
	this->lock->unlock(this->lock);

	return TRUE;
}

METHOD(listener_t, child_rekey, bool,
	private_counters_listener_t *this, ike_sa_t *ike_sa,
	child_sa_t *old, child_sa_t *new)
{
	this->lock->lock(this->lock);
	this->counters[COUNTER_CHILD_SA_REKEY]++;
	count_named(this, ike_sa, COUNTER_CHILD_SA_REKEY);
	this->lock->unlock(this->lock);

	return TRUE;
}

METHOD(listener_t, message_hook, bool,
	private_counters_listener_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	counter_type_t type;
	bool request;

	if ((incoming && !plain) || (!incoming && !plain))
	{	/* handle each message only once */
		return TRUE;
	}

	request = message->get_request(message);
	switch (message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
			if (incoming)
			{
				type = request ? COUNTER_IN_IKE_SA_INIT_REQ
							   : COUNTER_IN_IKE_SA_INIT_RSP;
			}
			else
			{
				type = request ? COUNTER_OUT_IKE_SA_INIT_REQ
							   : COUNTER_OUT_IKE_SA_INIT_RES;
			}
			break;
		case IKE_AUTH:
			if (incoming)
			{
				type = request ? COUNTER_IN_IKE_AUTH_REQ
							   : COUNTER_IN_IKE_AUTH_RSP;
			}
			else
			{
				type = request ? COUNTER_OUT_IKE_AUTH_REQ
							   : COUNTER_OUT_IKE_AUTH_RSP;
			}
			break;
		case CREATE_CHILD_SA:
			if (incoming)
			{
				type = request ? COUNTER_IN_CREATE_CHILD_SA_REQ
							   : COUNTER_IN_CREATE_CHILD_SA_RSP;
			}
			else
			{
				type = request ? COUNTER_OUT_CREATE_CHILD_SA_REQ
							   : COUNTER_OUT_CREATE_CHILD_SA_RSP;
			}
			break;
		case INFORMATIONAL:
			if (incoming)
			{
				type = request ? COUNTER_IN_INFORMATIONAL_REQ
							   : COUNTER_IN_INFORMATIONAL_RSP;
			}
			else
			{
				type = request ? COUNTER_OUT_INFORMATIONAL_REQ
							   : COUNTER_OUT_INFORMATIONAL_RSP;
			}
			break;
		default:
			return TRUE;
	}

	this->lock->lock(this->lock);
	this->counters[type]++;
	count_named(this, ike_sa, type);
	this->lock->unlock(this->lock);

	return TRUE;
}

CALLBACK(free_names, void,
	array_t * names)
{
	array_destroy_function(names, (void*)free, NULL);
}

METHOD(counters_query_t, get_names, enumerator_t*,
	private_counters_query_t *query)
{
	private_counters_listener_t *this = query->this;
	enumerator_t *enumerator;
	array_t *names;
	char *name;

	this->lock->lock(this->lock);
	names = array_create(0, this->conns->get_count(this->conns));
	enumerator = this->conns->create_enumerator(this->conns);
	while (enumerator->enumerate(enumerator, &name, NULL))
	{
		array_insert(names, ARRAY_TAIL, strdup(name));
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	array_sort(names, (void*)strcmp, NULL);

	return enumerator_create_cleaner(array_create_enumerator(names),
									 free_names, names);
}

METHOD(counters_query_t, get, bool,
	private_counters_query_t *query, counter_type_t type, char *name,
	uint64_t *value)
{
	private_counters_listener_t *this = query->this;
	uint64_t *counters = this->counters;

	this->lock->lock(this->lock);
	if (name)
	{
		entry_t *entry;

		entry = this->conns->get(this->conns, name);
		if (!entry)
		{
			this->lock->unlock(this->lock);
			return FALSE;
		}
		counters = entry->counters;
	}
	if (value)
	{
		*value = counters[type];
	}
	this->lock->unlock(this->lock);
	return TRUE;
}

METHOD(counters_query_t, get_all, uint64_t*,
	private_counters_query_t *query, char *name)
{
	private_counters_listener_t *this = query->this;
	entry_t *entry;
	uint64_t *result, *counters = this->counters;
	counter_type_t i;

	result = calloc(COUNTER_MAX, sizeof(uint64_t));

	this->lock->lock(this->lock);
	if (name)
	{
		entry = this->conns->get(this->conns, name);
		if (!entry)
		{
			this->lock->unlock(this->lock);
			free(result);
			return NULL;
		}
		counters = &entry->counters[0];
	}
	for (i = 0; i < countof(this->counters); i++)
	{
		result[i] = counters[i];
	}
	this->lock->unlock(this->lock);
	return result;
}

METHOD(counters_query_t, reset, void,
	private_counters_query_t *query, char *name)
{
	private_counters_listener_t *this = query->this;
	entry_t *entry = NULL;

	this->lock->lock(this->lock);
	if (name)
	{
		entry = this->conns->remove(this->conns, name);
	}
	else
	{
		memset(&this->counters, 0, sizeof(this->counters));
	}
	this->lock->unlock(this->lock);

	if (entry)
	{
		destroy_entry(entry);
	}
}

METHOD(counters_query_t, reset_all, void,
	private_counters_query_t *query)
{
	private_counters_listener_t *this = query->this;
	hashtable_t *new_conns, *conns;

	new_conns = hashtable_create((hashtable_hash_t)hash,
								 (hashtable_equals_t)equals, 4);

	this->lock->lock(this->lock);
	conns = this->conns;
	this->conns = new_conns;
	this->lock->unlock(this->lock);

	conns->destroy_function(conns, (void*)destroy_entry);
}

METHOD(counters_listener_t, destroy, void,
	private_counters_listener_t *this)
{
	lib->set(lib, "counters", NULL);

	this->conns->destroy_function(this->conns, (void*)destroy_entry);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * Described in header
 */
counters_listener_t *counters_listener_create()
{
	private_counters_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.alert = _alert,
				.ike_rekey = _ike_rekey,
				.child_rekey = _child_rekey,
				.message = _message_hook,
			},
			.destroy = _destroy,
		},
		.query = {
			.public = {
				.get_names = _get_names,
				.get = _get,
				.get_all = _get_all,
				.reset = _reset,
				.reset_all = _reset_all,
			},
		},
		.conns = hashtable_create((hashtable_hash_t)hash,
								  (hashtable_equals_t)equals, 4),
		.lock = spinlock_create(),
	);
	this->query.this = this;

	lib->set(lib, "counters", &this->query);

	return &this->public;
}
