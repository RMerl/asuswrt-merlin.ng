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

#include "dhcp_transaction.h"

#include <collections/linked_list.h>

typedef struct private_dhcp_transaction_t private_dhcp_transaction_t;

/**
 * Private data of an dhcp_transaction_t object.
 */
struct private_dhcp_transaction_t {

	/**
	 * Public dhcp_transaction_t interface.
	 */
	dhcp_transaction_t public;

	/**
	 * DHCP transaction ID
	 */
	u_int32_t id;

	/**
	 * Peer identity
	 */
	identification_t *identity;

	/**
	 * received DHCP address
	 */
	host_t *address;

	/**
	 * discovered DHCP server address
	 */
	host_t *server;

	/**
	 * List of added attributes, as attribute_entry_t
	 */
	linked_list_t *attributes;
};

/**
 * Entry for an added attribute
 */
typedef struct {
	configuration_attribute_type_t type;
	chunk_t data;
} attribute_entry_t;

METHOD(dhcp_transaction_t, get_id, u_int32_t,
	private_dhcp_transaction_t *this)
{
	return this->id;
}

METHOD(dhcp_transaction_t, get_identity, identification_t*,
	private_dhcp_transaction_t *this)
{
	return this->identity;
}

METHOD(dhcp_transaction_t, set_address, void,
	private_dhcp_transaction_t *this, host_t *address)
{
	DESTROY_IF(this->address);
	this->address = address;
}

METHOD(dhcp_transaction_t, get_address, host_t*,
	private_dhcp_transaction_t *this)
{
	return this->address;
}

METHOD(dhcp_transaction_t, set_server, void,
	private_dhcp_transaction_t *this, host_t *server)
{
	DESTROY_IF(this->server);
	this->server = server;
}

METHOD(dhcp_transaction_t, get_server, host_t*,
	private_dhcp_transaction_t *this)
{
	return this->server;
}

METHOD(dhcp_transaction_t, add_attribute, void,
	private_dhcp_transaction_t *this, configuration_attribute_type_t type,
	chunk_t data)
{
	attribute_entry_t *entry;

	INIT(entry,
		.type = type,
		.data = chunk_clone(data),
	);
	this->attributes->insert_last(this->attributes, entry);
}

/**
 * Filter function to map entries to type/data
 */
static bool attribute_filter(void *null, attribute_entry_t **entry,
							 configuration_attribute_type_t *type,
							 void **dummy, chunk_t *data)
{
	*type = (*entry)->type;
	*data = (*entry)->data;
	return TRUE;
}

METHOD(dhcp_transaction_t, create_attribute_enumerator, enumerator_t*,
	private_dhcp_transaction_t *this)
{
	return enumerator_create_filter(
						this->attributes->create_enumerator(this->attributes),
						(void*)attribute_filter, NULL, NULL);
}

/**
 * Clean up an attribute entry
 */
static void attribute_entry_destroy(attribute_entry_t *entry)
{
	free(entry->data.ptr);
	free(entry);
}

METHOD(dhcp_transaction_t, destroy, void,
	private_dhcp_transaction_t *this)
{
	this->identity->destroy(this->identity);
	DESTROY_IF(this->address);
	DESTROY_IF(this->server);
	this->attributes->destroy_function(this->attributes,
									   (void*)attribute_entry_destroy);
	free(this);
}

/**
 * See header
 */
dhcp_transaction_t *dhcp_transaction_create(u_int32_t id,
											identification_t *identity)
{
	private_dhcp_transaction_t *this;

	INIT(this,
		.public = {
			.get_id = _get_id,
			.get_identity = _get_identity,
			.set_address = _set_address,
			.get_address = _get_address,
			.set_server = _set_server,
			.get_server = _get_server,
			.add_attribute = _add_attribute,
			.create_attribute_enumerator = _create_attribute_enumerator,
			.destroy = _destroy,
		},
		.id = id,
		.identity = identity->clone(identity),
		.attributes = linked_list_create(),
	);

	return &this->public;
}

