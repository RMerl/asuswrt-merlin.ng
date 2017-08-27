/*
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup gateway gateway
 * @{ @ingroup manager
 */

#ifndef GATEWAY_H_
#define GATEWAY_H_

#include <networking/host.h>
#include <collections/enumerator.h>

typedef struct gateway_t gateway_t;

/**
 * A connection to a gateway.
 */
struct gateway_t {

	/**
	 * Send an XML request to the gateway.
	 *
	 * @param xml		xml request string
	 * @param ...		printf style argument list for xml request string
	 * @return			allocated xml response string
	 */
	char* (*request)(gateway_t *this, char *xml, ...);

	/**
	 * Query the list of IKE_SAs and all its children.
	 *
	 * @return			enumerator over ikesa XML elements
	 */
	enumerator_t* (*query_ikesalist)(gateway_t *this);

	/**
	 * Query the list of peer configs and its subconfigs.
	 *
	 * @return			enumerator over peerconfig XML elements
	 */
	enumerator_t* (*query_configlist)(gateway_t *this);

	/**
	 * Terminate an IKE or a CHILD SA.
	 *
	 * @param ike		TRUE for IKE-, FALSE for a CHILD-SA
	 * @param id		ID of the SA to terminate
	 * @return			enumerator over control response XML children
	 */
	enumerator_t* (*terminate)(gateway_t *this, bool ike, u_int32_t id);

	/**
	 * Initiate an IKE or a CHILD SA.
	 *
	 * @param ike		TRUE for IKE-, FALSE for CHILD-SA
	 * @param name		name of the peer/child config
	 * @return			enumerator over control response XML children
	 */
	enumerator_t* (*initiate)(gateway_t *this, bool ike, char *name);

	/**
	 * Destroy a gateway instance.
	 */
	void (*destroy)(gateway_t *this);
};

/**
 * Create a gateway instance using a TCP connection.
 *
 * @param name			name of the gateway
 * @param host			gateway connection endpoint
 * @return				gateway connection
 */
gateway_t *gateway_create_tcp(char *name, host_t *host);

/**
 * Create a gateway instance using a UNIX socket.
 *
 * @param name			name of the gateway
 * @return				gateway connection
 */
gateway_t *gateway_create_unix(char *name);

#endif /** GATEWAY_H_ @}*/
