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

/**
 * @defgroup dhcp_transaction dhcp_transaction
 * @{ @ingroup dhcp
 */

#ifndef DHCP_TRANSACTION_H_
#define DHCP_TRANSACTION_H_

#include <networking/host.h>
#include <utils/identification.h>
#include <attributes/attributes.h>

typedef struct dhcp_transaction_t dhcp_transaction_t;

/**
 * DHCP transaction class.
 */
struct dhcp_transaction_t {

	/**
	 * Get the DHCP transaction ID.
	 *
	 * @return			DHCP transaction identifier
	 */
	uint32_t (*get_id)(dhcp_transaction_t *this);

	/**
	 * Get the peer identity this transaction is used for.
	 *
	 * @return			peer Identity
	 */
	identification_t* (*get_identity)(dhcp_transaction_t *this);

	/**
	 * Set the DHCP address received using this transaction.
	 *
	 * @param host		received DHCP address
	 */
	void (*set_address)(dhcp_transaction_t *this, host_t *address);

	/**
	 * Get the DHCP address received using this transaction.
	 *
	 * @return			received DHCP address
	 */
	host_t* (*get_address)(dhcp_transaction_t *this);

	/**
	 * Set the DHCP server address discovered.
	 *
	 * @param server	DHCP server address
	 */
	void (*set_server)(dhcp_transaction_t *this, host_t *server);

	/**
	 * Get the DHCP server address.
	 *
	 * @return			DHCP server address
	 */
	host_t* (*get_server)(dhcp_transaction_t *this);

	/**
	 * Add an additional attribute to serve to peer.
	 *
	 * @param type		type of attribute
	 * @param data		attribute data
	 */
	void (*add_attribute)(dhcp_transaction_t *this,
						  configuration_attribute_type_t type, chunk_t data);

	/**
	 * Create an enumerator over added attributes.
	 *
	 * @return			enumerator over (configuration_attribute_t, chunk_t)
	 */
	enumerator_t* (*create_attribute_enumerator)(dhcp_transaction_t *this);

	/**
	 * Destroy a dhcp_transaction_t.
	 */
	void (*destroy)(dhcp_transaction_t *this);
};

/**
 * Create a dhcp_transaction instance.
 *
 * @param id		DHCP transaction identifier
 * @param identity	peer identity this transaction is used for
 * @return			transaction instance
 */
dhcp_transaction_t *dhcp_transaction_create(uint32_t id,
											identification_t *identity);

#endif /** DHCP_TRANSACTION_H_ @}*/
