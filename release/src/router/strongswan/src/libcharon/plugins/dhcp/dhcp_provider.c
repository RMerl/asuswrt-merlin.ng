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

#include "dhcp_provider.h"

#include <collections/hashtable.h>
#include <threading/mutex.h>

typedef struct private_dhcp_provider_t private_dhcp_provider_t;

/**
 * Private data of an dhcp_provider_t object.
 */
struct private_dhcp_provider_t {

	/**
	 * Public dhcp_provider_t interface.
	 */
	dhcp_provider_t public;

	/**
	 * Completed DHCP transactions
	 */
	hashtable_t *transactions;

	/**
	 * Lock for transactions
	 */
	mutex_t *mutex;

	/**
	 * DHCP communication socket
	 */
	dhcp_socket_t *socket;
};

/**
 * Hash ID and host to a key
 */
static uintptr_t hash_id_host(identification_t *id, host_t *host)
{
	return chunk_hash_inc(id->get_encoding(id),
						  chunk_hash(host->get_address(host)));
}

/**
 * Hash a DHCP transaction to a key, using address and id
 */
static uintptr_t hash_transaction(dhcp_transaction_t *transaction)
{
	return hash_id_host(transaction->get_identity(transaction),
						transaction->get_address(transaction));
}

METHOD(attribute_provider_t, acquire_address, host_t*,
	private_dhcp_provider_t *this, linked_list_t *pools,
	ike_sa_t *ike_sa, host_t *requested)
{
	dhcp_transaction_t *transaction, *old;
	enumerator_t *enumerator;
	identification_t *id;
	char *pool;
	host_t *vip = NULL;

	if (requested->get_family(requested) != AF_INET)
	{
		return NULL;
	}
	id = ike_sa->get_other_eap_id(ike_sa);
	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &pool))
	{
		if (!streq(pool, "dhcp"))
		{
			continue;
		}
		transaction = this->socket->enroll(this->socket, id);
		if (!transaction)
		{
			continue;
		}
		vip = transaction->get_address(transaction);
		vip = vip->clone(vip);
		this->mutex->lock(this->mutex);
		old = this->transactions->put(this->transactions,
							(void*)hash_transaction(transaction), transaction);
		this->mutex->unlock(this->mutex);
		DESTROY_IF(old);
		break;
	}
	enumerator->destroy(enumerator);
	return vip;
}

METHOD(attribute_provider_t, release_address, bool,
	private_dhcp_provider_t *this, linked_list_t *pools,
	host_t *address, ike_sa_t *ike_sa)
{
	dhcp_transaction_t *transaction;
	enumerator_t *enumerator;
	identification_t *id;
	bool found = FALSE;
	char *pool;

	if (address->get_family(address) != AF_INET)
	{
		return FALSE;
	}
	id = ike_sa->get_other_eap_id(ike_sa);
	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &pool))
	{
		if (!streq(pool, "dhcp"))
		{
			continue;
		}
		this->mutex->lock(this->mutex);
		transaction = this->transactions->remove(this->transactions,
										(void*)hash_id_host(id, address));
		this->mutex->unlock(this->mutex);
		if (transaction)
		{
			this->socket->release(this->socket, transaction);
			transaction->destroy(transaction);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

METHOD(attribute_provider_t, create_attribute_enumerator, enumerator_t*,
	private_dhcp_provider_t *this, linked_list_t *pools, ike_sa_t *ike_sa,
	linked_list_t *vips)
{
	dhcp_transaction_t *transaction = NULL;
	enumerator_t *enumerator;
	identification_t *id;
	host_t *vip;

	if (!pools->find_first(pools, linked_list_match_str, NULL, "dhcp"))
	{
		return NULL;
	}

	id = ike_sa->get_other_eap_id(ike_sa);
	this->mutex->lock(this->mutex);
	enumerator = vips->create_enumerator(vips);
	while (enumerator->enumerate(enumerator, &vip))
	{
		transaction = this->transactions->get(this->transactions,
											  (void*)hash_id_host(id, vip));
		if (transaction)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!transaction)
	{
		this->mutex->unlock(this->mutex);
		return NULL;
	}
	return enumerator_create_cleaner(
						transaction->create_attribute_enumerator(transaction),
						(void*)this->mutex->unlock, this->mutex);
}

METHOD(dhcp_provider_t, destroy, void,
	private_dhcp_provider_t *this)
{
	enumerator_t *enumerator;
	dhcp_transaction_t *value;
	void *key;

	enumerator = this->transactions->create_enumerator(this->transactions);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		value->destroy(value);
	}
	enumerator->destroy(enumerator);
	this->transactions->destroy(this->transactions);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
dhcp_provider_t *dhcp_provider_create(dhcp_socket_t *socket)
{
	private_dhcp_provider_t *this;

	INIT(this,
		.public = {
			.provider = {
				.acquire_address = _acquire_address,
				.release_address = _release_address,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.destroy = _destroy,
		},
		.socket = socket,
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.transactions = hashtable_create(hashtable_hash_ptr,
										 hashtable_equals_ptr, 8),
	);

	return &this->public;
}
