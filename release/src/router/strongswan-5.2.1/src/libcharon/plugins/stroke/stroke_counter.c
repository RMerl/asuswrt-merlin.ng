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

#include "stroke_counter.h"

#include <threading/spinlock.h>
#include <collections/hashtable.h>

ENUM(stroke_counter_type_names,
	COUNTER_INIT_IKE_SA_REKEY, COUNTER_OUT_INFORMATIONAL_RSP,
	"ikeInitRekey",
	"ikeRspRekey",
	"ikeChildSaRekey",
	"ikeInInvalid",
	"ikeInInvalidSpi",
	"ikeInInitReq",
	"ikeInInitRsp",
	"ikeOutInitReq",
	"ikeOutInitRsp",
	"ikeInAuthReq",
	"ikeInAuthRsp",
	"ikeOutAuthReq",
	"ikeOutAuthRsp",
	"ikeInCrChildReq",
	"ikeInCrChildRsp",
	"ikeOutCrChildReq",
	"ikeOutCrChildRsp",
	"ikeInInfoReq",
	"ikeInInfoRsp",
	"ikeOutInfoReq",
	"ikeOutInfoRsp",
);

typedef struct private_stroke_counter_t private_stroke_counter_t;

/**
 * Private data of an stroke_counter_t object.
 */
struct private_stroke_counter_t {

	/**
	 * Public stroke_counter_t interface.
	 */
	stroke_counter_t public;

	/**
	 * Global counter values
	 */
	u_int64_t counter[COUNTER_MAX];

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
	u_int64_t counter[COUNTER_MAX];
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
static void count_named(private_stroke_counter_t *this,
						ike_sa_t *ike_sa, stroke_counter_type_t type)
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
		entry->counter[type]++;
	}
}

METHOD(listener_t, alert, bool,
	private_stroke_counter_t *this, ike_sa_t *ike_sa,
	alert_t alert, va_list args)
{
	stroke_counter_type_t type;

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
	this->counter[type]++;
	count_named(this, ike_sa, type);
	this->lock->unlock(this->lock);

	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	private_stroke_counter_t *this, ike_sa_t *old, ike_sa_t *new)
{
	stroke_counter_type_t type;
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
	this->counter[type]++;
	count_named(this, old, type);
	this->lock->unlock(this->lock);

	return TRUE;
}

METHOD(listener_t, child_rekey, bool,
	private_stroke_counter_t *this, ike_sa_t *ike_sa,
	child_sa_t *old, child_sa_t *new)
{
	this->lock->lock(this->lock);
	this->counter[COUNTER_CHILD_SA_REKEY]++;
	count_named(this, ike_sa, COUNTER_CHILD_SA_REKEY);
	this->lock->unlock(this->lock);

	return TRUE;
}

METHOD(listener_t, message_hook, bool,
	private_stroke_counter_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	stroke_counter_type_t type;
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
	this->counter[type]++;
	count_named(this, ike_sa, type);
	this->lock->unlock(this->lock);

	return TRUE;
}

/**
 * Print a single counter value to out
 */
static void print_counter(FILE *out, stroke_counter_type_t type,
						  u_int64_t counter)
{
	fprintf(out, "%-18N %12llu\n", stroke_counter_type_names, type, counter);
}

/**
 * Print IKE counters for a specific connection
 */
static void print_one(private_stroke_counter_t *this, FILE *out, char *name)
{
	u_int64_t counter[COUNTER_MAX];
	entry_t *entry;
	int i;

	this->lock->lock(this->lock);
	entry = this->conns->get(this->conns, name);
	if (entry)
	{
		for (i = 0; i < countof(this->counter); i++)
		{
			counter[i] = entry->counter[i];
		}
	}
	this->lock->unlock(this->lock);

	if (entry)
	{
		fprintf(out, "\nList of IKE counters for '%s':\n\n", name);
		for (i = 0; i < countof(this->counter); i++)
		{
			print_counter(out, i, counter[i]);
		}
	}
	else
	{
		fprintf(out, "No IKE counters found for '%s'\n", name);
	}
}

/**
 * Print counters for all connections
 */
static void print_all(private_stroke_counter_t *this, FILE *out)
{
	enumerator_t *enumerator;
	entry_t *entry;
	linked_list_t *list;
	char *name;

	list = linked_list_create();

	this->lock->lock(this->lock);
	enumerator = this->conns->create_enumerator(this->conns);
	while (enumerator->enumerate(enumerator, &name, &entry))
	{
		list->insert_last(list, strdup(name));
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &name))
	{
		print_one(this, out, name);
	}
	enumerator->destroy(enumerator);

	list->destroy_function(list, free);
}

/**
 * Print global counters
 */
static void print_global(private_stroke_counter_t *this, FILE *out)
{
	u_int64_t counter[COUNTER_MAX];
	int i;

	this->lock->lock(this->lock);
	for (i = 0; i < countof(this->counter); i++)
	{
		counter[i] = this->counter[i];
	}
	this->lock->unlock(this->lock);

	fprintf(out, "\nList of IKE counters:\n\n");

	for (i = 0; i < countof(this->counter); i++)
	{
		print_counter(out, i, counter[i]);
	}
}

METHOD(stroke_counter_t, print, void,
	private_stroke_counter_t *this, FILE *out, char *name)
{
	if (name)
	{
		if (streq(name, "all"))
		{
			return print_all(this, out);
		}
		return print_one(this, out, name);
	}
	return print_global(this, out);
}

METHOD(stroke_counter_t, reset, void,
	private_stroke_counter_t *this, char *name)
{
	this->lock->lock(this->lock);
	if (name)
	{
		entry_t *entry;

		entry = this->conns->remove(this->conns, name);
		if (entry)
		{
			destroy_entry(entry);
		}
	}
	else
	{
		memset(&this->counter, 0, sizeof(this->counter));
	}
	this->lock->unlock(this->lock);
}

METHOD(stroke_counter_t, destroy, void,
	private_stroke_counter_t *this)
{
	enumerator_t *enumerator;
	char *name;
	entry_t *entry;

	enumerator = this->conns->create_enumerator(this->conns);
	while (enumerator->enumerate(enumerator, &name, &entry))
	{
		destroy_entry(entry);
	}
	enumerator->destroy(enumerator);
	this->conns->destroy(this->conns);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
stroke_counter_t *stroke_counter_create()
{
	private_stroke_counter_t *this;

	INIT(this,
		.public = {
			.listener = {
				.alert = _alert,
				.ike_rekey = _ike_rekey,
				.child_rekey = _child_rekey,
				.message = _message_hook,
			},
			.print = _print,
			.reset = _reset,
			.destroy = _destroy,
		},
		.conns = hashtable_create((hashtable_hash_t)hash,
								  (hashtable_equals_t)equals, 4),
		.lock = spinlock_create(),
	);

	return &this->public;
}
