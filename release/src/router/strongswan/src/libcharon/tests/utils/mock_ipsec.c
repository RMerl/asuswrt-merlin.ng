/*
 * Copyright (C) 2016-2017 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#include "mock_ipsec.h"

#include <daemon.h>
#include <collections/hashtable.h>
#include <collections/array.h>

#include <assert.h>

typedef struct private_kernel_ipsec_t private_kernel_ipsec_t;

/**
 * Private data
 */
struct private_kernel_ipsec_t {

	/**
	 * Public interface
	 */
	kernel_ipsec_t public;

	/**
	 * Rekey listener
	 */
	listener_t listener;

	/**
	 * Allocated SPI
	 */
	refcount_t spi;

	/**
	 * Installed SAs
	 */
	hashtable_t *sas;
};

/**
 * Global instance
 */
static private_kernel_ipsec_t *instance;

/**
 * Data about installed IPsec SAs
 */
typedef struct {
	/**
	 * SPI of the SA
	 */
	uint32_t spi;

	/**
	 * Associated IKE_SA
	 */
	ike_sa_t *ike_sa;

	/**
	 * TRUE if this was an allocated SPI
	 */
	bool alloc;

} entry_t;

/**
 * Hash an IPsec SA entry
 */
static u_int entry_hash(const void *key)
{
	entry_t *entry = (entry_t*)key;
	return chunk_hash_inc(chunk_from_thing(entry->spi),
						  chunk_hash(chunk_from_thing(entry->ike_sa)));
}

/**
 * Compare an IPsec SA entry
 */
static bool entry_equals(const void *key, const void *other_key)
{
	entry_t *a = (entry_t*)key, *b = (entry_t*)other_key;
	return a->spi == b->spi && a->ike_sa == b->ike_sa;
}

METHOD(kernel_ipsec_t, get_spi, status_t,
	private_kernel_ipsec_t *this, host_t *src, host_t *dst, uint8_t protocol,
	uint32_t *spi)
{
	entry_t *entry;

	*spi = (uint32_t)ref_get(&this->spi);
	INIT(entry,
		.spi = *spi,
		.ike_sa = charon->bus->get_sa(charon->bus),
		.alloc = TRUE,
	);
	entry = this->sas->put(this->sas, entry, entry);
	assert(!entry);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_kernel_ipsec_t *this, host_t *src, host_t *dst, uint16_t *cpi)
{
	return FAILED;
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_kernel_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_add_sa_t *data)
{
	entry_t *entry;

	INIT(entry,
		.spi = id->spi,
		.ike_sa = charon->bus->get_sa(charon->bus),
	);
	if (data->inbound)
	{
		entry = this->sas->put(this->sas, entry, entry);
		assert(entry && entry->alloc);
		free(entry);
	}
	else
	{
		entry = this->sas->put(this->sas, entry, entry);
		assert(!entry);
	}
	return SUCCESS;
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_kernel_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_update_sa_t *data)
{
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_kernel_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_query_sa_t *data, uint64_t *bytes, uint64_t *packets,
	time_t *time)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_kernel_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_del_sa_t *data)
{
	entry_t *entry, lookup = {
		.spi = id->spi,
		.ike_sa = charon->bus->get_sa(charon->bus),
	};

	entry = this->sas->remove(this->sas, &lookup);
	assert(entry);
	free(entry);
	return SUCCESS;
}

METHOD(listener_t, ike_rekey, bool,
	listener_t *listener, ike_sa_t *old, ike_sa_t *new)
{
	enumerator_t *enumerator;
	array_t *sas = NULL;
	entry_t *entry;

	enumerator = instance->sas->create_enumerator(instance->sas);
	while (enumerator->enumerate(enumerator, &entry, NULL))
	{
		if (entry->ike_sa == old)
		{
			instance->sas->remove_at(instance->sas, enumerator);
			array_insert_create(&sas, ARRAY_TAIL, entry);
		}
	}
	enumerator->destroy(enumerator);
	enumerator = array_create_enumerator(sas);
	while (enumerator->enumerate(enumerator, &entry))
	{
		array_remove_at(sas, enumerator);
		entry->ike_sa = new;
		entry = instance->sas->put(instance->sas, entry, entry);
		assert(!entry);
	}
	enumerator->destroy(enumerator);
	array_destroy(sas);
	return TRUE;
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_kernel_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_kernel_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_query_policy_t *data, time_t *use_time)
{
	*use_time = 1;
	return SUCCESS;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_kernel_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	return SUCCESS;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_kernel_ipsec_t *this)
{
	charon->bus->remove_listener(charon->bus, &this->listener);
	this->sas->destroy(this->sas);
	free(this);
}

/*
 * Described in header
 */
kernel_ipsec_t *mock_ipsec_create()
{
	private_kernel_ipsec_t *this;

	INIT(this,
		.public = {
			.get_spi = _get_spi,
			.get_cpi = _get_cpi,
			.add_sa = _add_sa,
			.update_sa = _update_sa,
			.query_sa = _query_sa,
			.del_sa = _del_sa,
			.flush_sas = (void*)return_failed,
			.add_policy = _add_policy,
			.query_policy = _query_policy,
			.del_policy = _del_policy,
			.flush_policies = (void*)return_failed,
			.bypass_socket = (void*)return_true,
			.enable_udp_decap = (void*)return_true,
			.destroy = _destroy,
		},
		.listener = {
			.ike_rekey = _ike_rekey,
		},
		.sas = hashtable_create(entry_hash, entry_equals, 8),
	);

	instance = this;

	charon->bus->add_listener(charon->bus, &this->listener);

	return &this->public;
}


CALLBACK(filter_sas, bool,
	void *data, enumerator_t *orig, va_list args)
{
	entry_t *entry;
	ike_sa_t **ike_sa;
	uint32_t *spi;

	VA_ARGS_VGET(args, ike_sa, spi);

	while (orig->enumerate(orig, &entry, NULL))
	{
		if (entry->alloc)
		{
			continue;
		}
		*ike_sa = entry->ike_sa;
		*spi = entry->spi;
		return TRUE;
	}
	return FALSE;
}

/*
 * Described in header
 */
enumerator_t *mock_ipsec_create_sa_enumerator()
{
	return enumerator_create_filter(
							instance->sas->create_enumerator(instance->sas),
							filter_sas, NULL, NULL);
}
