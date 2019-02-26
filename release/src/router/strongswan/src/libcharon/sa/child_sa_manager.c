/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#include "child_sa_manager.h"

#include <daemon.h>
#include <threading/mutex.h>
#include <collections/hashtable.h>

typedef struct private_child_sa_manager_t private_child_sa_manager_t;

/**
 * Private data of an child_sa_manager_t object.
 */
struct private_child_sa_manager_t {

	/**
	 * Public child_sa_manager_t interface.
	 */
	child_sa_manager_t public;

	/**
	 * CHILD_SAs by inbound SPI/dst, child_entry_t => child_entry_t
	 */
	hashtable_t *in;

	/**
	 * CHILD_SAs by outbound SPI/dst, child_entry_t => child_entry_t
	 */
	hashtable_t *out;

	/**
	 * CHILD_SAs by unique ID, child_entry_t => child_entry_t
	 */
	hashtable_t *ids;

	/**
	 * Mutex to access any hashtable
	 */
	mutex_t *mutex;
};

/**
 * Hashtable entry for a known CHILD_SA
 */
typedef struct {
	/** the associated IKE_SA */
	ike_sa_id_t *ike_id;
	/** unique CHILD_SA identifier */
	uint32_t unique_id;
	/** inbound SPI */
	uint32_t spi_in;
	/** outbound SPI */
	uint32_t spi_out;
	/** inbound host address */
	host_t *host_in;
	/** outbound host address and port */
	host_t *host_out;
	/** IPsec protocol, AH|ESP */
	protocol_id_t proto;
} child_entry_t;

/**
 * Destroy a CHILD_SA entry
 */
static void child_entry_destroy(child_entry_t *entry)
{
	entry->ike_id->destroy(entry->ike_id);
	entry->host_in->destroy(entry->host_in);
	entry->host_out->destroy(entry->host_out);
	free(entry);
}

/**
 * Hashtable hash function for inbound SAs
 */
static u_int hash_in(child_entry_t *entry)
{
	return chunk_hash_inc(chunk_from_thing(entry->spi_in),
			chunk_hash_inc(entry->host_in->get_address(entry->host_in),
			 chunk_hash(chunk_from_thing(entry->proto))));
}

/**
 * Hashtable equals function for inbound SAs
 */
static bool equals_in(child_entry_t *a, child_entry_t *b)
{
	return a->spi_in == b->spi_in &&
		   a->proto == b->proto &&
		   a->host_in->ip_equals(a->host_in, b->host_in);
}

/**
 * Hashtable hash function for outbound SAs
 */
static u_int hash_out(child_entry_t *entry)
{
	return chunk_hash_inc(chunk_from_thing(entry->spi_out),
			chunk_hash_inc(entry->host_out->get_address(entry->host_out),
			 chunk_hash(chunk_from_thing(entry->proto))));
}

/**
 * Hashtable equals function for outbound SAs
 */
static bool equals_out(child_entry_t *a, child_entry_t *b)
{
	return a->spi_out == b->spi_out &&
		   a->proto == b->proto &&
		   a->host_out->ip_equals(a->host_out, b->host_out);
}

/**
 * Hashtable hash function for SAs by unique ID
 */
static u_int hash_id(child_entry_t *entry)
{
	return chunk_hash(chunk_from_thing(entry->unique_id));
}

/**
 * Hashtable equals function for SAs by unique ID
 */
static bool equals_id(child_entry_t *a, child_entry_t *b)
{
	return a->unique_id == b->unique_id;
}

METHOD(child_sa_manager_t, add, void,
	private_child_sa_manager_t *this, child_sa_t *child_sa, ike_sa_t *ike_sa)
{
	child_entry_t *entry;
	host_t *in, *out;
	ike_sa_id_t *id;

	id = ike_sa->get_id(ike_sa);
	in = ike_sa->get_my_host(ike_sa);
	out = ike_sa->get_other_host(ike_sa);

	INIT(entry,
		.ike_id = id->clone(id),
		.unique_id = child_sa->get_unique_id(child_sa),
		.proto = child_sa->get_protocol(child_sa),
		.spi_in = child_sa->get_spi(child_sa, TRUE),
		.spi_out = child_sa->get_spi(child_sa, FALSE),
		.host_in = in->clone(in),
		.host_out = out->clone(out),
	);

	this->mutex->lock(this->mutex);
	if (!this->in->get(this->in, entry) &&
		!this->out->get(this->out, entry))
	{
		this->in->put(this->in, entry, entry);
		this->out->put(this->out, entry, entry);
		entry = this->ids->put(this->ids, entry, entry);
	}
	this->mutex->unlock(this->mutex);

	if (entry)
	{
		child_entry_destroy(entry);
	}
}

METHOD(child_sa_manager_t, remove_, void,
	private_child_sa_manager_t *this, child_sa_t *child_sa)
{
	child_entry_t *entry, key = {
		.unique_id = child_sa->get_unique_id(child_sa),
	};

	this->mutex->lock(this->mutex);
	entry = this->ids->remove(this->ids, &key);
	if (entry)
	{
		this->in->remove(this->in, entry);
		this->out->remove(this->out, entry);
	}
	this->mutex->unlock(this->mutex);

	if (entry)
	{
		child_entry_destroy(entry);
	}
}

/**
 * Check out an IKE_SA for a given CHILD_SA
 */
static ike_sa_t *checkout_ikesa(private_child_sa_manager_t *this,
					ike_sa_id_t *id, uint32_t unique_id, child_sa_t **child_sa)
{
	enumerator_t *enumerator;
	child_sa_t *current;
	ike_sa_t *ike_sa;
	bool found = FALSE;

	ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager, id);
	id->destroy(id);
	if (ike_sa)
	{
		enumerator = ike_sa->create_child_sa_enumerator(ike_sa);
		while (enumerator->enumerate(enumerator, &current))
		{
			found = current->get_unique_id(current) == unique_id;
			if (found)
			{
				if (child_sa)
				{
					*child_sa = current;
				}
				break;
			}
		}
		enumerator->destroy(enumerator);

		if (found)
		{
			return ike_sa;
		}
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	}
	return NULL;
}

METHOD(child_sa_manager_t, checkout_by_id, ike_sa_t*,
	private_child_sa_manager_t *this, uint32_t unique_id,
	child_sa_t **child_sa)
{
	ike_sa_id_t *id;
	child_entry_t *entry, key = {
		.unique_id = unique_id,
	};

	this->mutex->lock(this->mutex);
	entry = this->ids->get(this->ids, &key);
	if (entry)
	{
		id = entry->ike_id->clone(entry->ike_id);
	}
	this->mutex->unlock(this->mutex);

	if (entry)
	{
		return checkout_ikesa(this, id, unique_id, child_sa);
	}
	return NULL;
}

METHOD(child_sa_manager_t, checkout, ike_sa_t*,
	private_child_sa_manager_t *this, protocol_id_t protocol, uint32_t spi,
	host_t *dst, child_sa_t **child_sa)
{
	ike_sa_id_t *id;
	uint32_t unique_id;
	child_entry_t *entry, key = {
		.spi_in = spi,
		.spi_out = spi,
		.host_in = dst,
		.host_out = dst,
		.proto = protocol,
	};

	this->mutex->lock(this->mutex);
	entry = this->in->get(this->in, &key);
	if (!entry)
	{
		entry = this->out->get(this->out, &key);
	}
	if (entry)
	{
		unique_id = entry->unique_id;
		id = entry->ike_id->clone(entry->ike_id);
	}
	this->mutex->unlock(this->mutex);

	if (entry)
	{
		return checkout_ikesa(this, id, unique_id, child_sa);
	}
	return NULL;
}

METHOD(child_sa_manager_t, destroy, void,
	private_child_sa_manager_t *this)
{
	this->in->destroy(this->in);
	this->out->destroy(this->out);
	this->ids->destroy(this->ids);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
child_sa_manager_t *child_sa_manager_create()
{
	private_child_sa_manager_t *this;

	INIT(this,
		.public = {
			.add = _add,
			.remove = _remove_,
			.checkout = _checkout,
			.checkout_by_id = _checkout_by_id,
			.destroy = _destroy,
		},
		.in = hashtable_create((hashtable_hash_t)hash_in,
							   (hashtable_equals_t)equals_in, 8),
		.out = hashtable_create((hashtable_hash_t)hash_out,
							   (hashtable_equals_t)equals_out, 8),
		.ids = hashtable_create((hashtable_hash_t)hash_id,
							   (hashtable_equals_t)equals_id, 8),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	return &this->public;
}
