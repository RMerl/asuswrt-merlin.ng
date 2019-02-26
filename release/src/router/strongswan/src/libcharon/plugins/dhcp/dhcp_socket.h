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
 * @defgroup dhcp_socket dhcp_socket
 * @{ @ingroup dhcp
 */

#ifndef DHCP_SOCKET_H_
#define DHCP_SOCKET_H_

typedef struct dhcp_socket_t dhcp_socket_t;

#include "dhcp_transaction.h"

/**
 * DHCP socket implementation
 */
struct dhcp_socket_t {

	/**
	 * Enroll a client address using DHCP.
	 *
	 * @param identity		peer identity to enroll an address for
	 * @return				completed DHCP transaction, NULL on failure
	 */
	dhcp_transaction_t* (*enroll)(dhcp_socket_t *this,
								  identification_t *identity);

	/**
	 * Release an enrolled DHCP address.
	 *
	 * @param transaction	transaction returned by enroll
	 */
	void (*release)(dhcp_socket_t *this, dhcp_transaction_t *transaction);

	/**
	 * Destroy a dhcp_socket_t.
	 */
	void (*destroy)(dhcp_socket_t *this);
};

/**
 * Create a dhcp_socket instance.
 */
dhcp_socket_t *dhcp_socket_create();

#endif /** DHCP_SOCKET_H_ @}*/
