/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup sasl sasl
 * @ingroup pt_tls
 *
 * @defgroup sasl_mechanism sasl_mechanism
 * @{ @ingroup sasl
 */

#ifndef SASL_MECHANISM_H_
#define SASL_MECHANISM_H_

typedef struct sasl_mechanism_t sasl_mechanism_t;

#include <library.h>

/**
 * Constructor function for SASL mechansims.
 *
 * @param name			name of the requested SASL mechanism
 * @param client		client identity, NULL to act as server
 * @return				SASL mechanism, NULL on failure
 */
typedef sasl_mechanism_t*(*sasl_mechanism_constructor_t)(char *name,
													identification_t *client);

/**
 * Generic interface for SASL mechanisms.
 */
struct sasl_mechanism_t {

	/**
	 * Get the name of this SASL mechanism.
	 *
	 * @return			name of SASL mechanism
	 */
	char* (*get_name)(sasl_mechanism_t *this);

	/**
	 * Get the client identity
	 *
	 * @return			client identity
	 */
	identification_t* (*get_client)(sasl_mechanism_t *this);

	/**
	 * Build a SASL message to send to remote host.
	 *
	 * A message is returned if the return value is NEED_MORE or SUCCESS. A
	 * client MUST NOT return SUCCESS in build(), as the final message
	 * is always from server to client (even if it is an empty result message).
	 *
	 * @param message	receives allocated SASL message, to free
	 * @return
	 *					- FAILED if mechanism failed
	 *					- NEED_MORE if additional exchanges required
	 *					- INVALID_STATE if currently nothing to build
	 *					- SUCCESS if mechanism authenticated successfully
	 */
	status_t (*build)(sasl_mechanism_t *this, chunk_t *message);

	/**
	 * Process a SASL message received from remote host.
	 *
	 * If a server returns SUCCESS during process(), an empty result message
	 * is sent to complete the SASL exchange.
	 *
	 * @param message	received SASL message to process
	 * @return
	 *					- FAILED if mechanism failed
	 *					- NEED_MORE if additional exchanges required
	 *					- SUCCESS if mechanism authenticated successfully
	 */
	status_t (*process)(sasl_mechanism_t *this, chunk_t message);

	/**
	 * Destroy a sasl_mechanism_t.
	 */
	void (*destroy)(sasl_mechanism_t *this);
};

/**
 * Create a sasl_mechanism instance.
 *
 * @param name			name of SASL mechanism to create
 * @param client		client identity, NULL to act as server
 * @return				SASL mechanism instance, NULL if not found
 */
sasl_mechanism_t *sasl_mechanism_create(char *name, identification_t *client);

/**
 * Create an enumerator over supported SASL mechanism names.
 *
 * @param server		TRUE for server instance, FALSE for client
 * @return				enumerator over char*
 */
enumerator_t* sasl_mechanism_create_enumerator(bool server);

#endif /** SASL_MECHANISM_H_ @}*/
