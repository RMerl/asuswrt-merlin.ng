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

/**
 * @defgroup counters_query counters_query
 * @{ @ingroup counters
 */

#ifndef COUNTERS_QUERY_H_
#define COUNTERS_QUERY_H_

#include <bus/listeners/listener.h>

typedef struct counters_query_t counters_query_t;
typedef enum counter_type_t counter_type_t;

enum counter_type_t {
	/** initiated IKE_SA rekeyings */
	COUNTER_INIT_IKE_SA_REKEY,
	/** responded IKE_SA rekeyings */
	COUNTER_RESP_IKE_SA_REKEY,
	/** completed CHILD_SA rekeyings */
	COUNTER_CHILD_SA_REKEY,
	/** messages with invalid types, length, or a value out of range */
	COUNTER_IN_INVALID,
	/** messages with an invalid IKE SPI */
	COUNTER_IN_INVALID_IKE_SPI,
	/** received IKE_SA_INIT requests */
	COUNTER_IN_IKE_SA_INIT_REQ,
	/** received IKE_SA_INIT responses */
	COUNTER_IN_IKE_SA_INIT_RSP,
	/** sent IKE_SA_INIT requests */
	COUNTER_OUT_IKE_SA_INIT_REQ,
	/** sent IKE_SA_INIT responses */
	COUNTER_OUT_IKE_SA_INIT_RES,
	/** received IKE_AUTH requests */
	COUNTER_IN_IKE_AUTH_REQ,
	/** received IKE_AUTH responses */
	COUNTER_IN_IKE_AUTH_RSP,
	/** sent IKE_AUTH requests */
	COUNTER_OUT_IKE_AUTH_REQ,
	/** sent IKE_AUTH responses */
	COUNTER_OUT_IKE_AUTH_RSP,
	/** received CREATE_CHILD_SA requests */
	COUNTER_IN_CREATE_CHILD_SA_REQ,
	/** received CREATE_CHILD_SA responses */
	COUNTER_IN_CREATE_CHILD_SA_RSP,
	/** sent CREATE_CHILD_SA requests */
	COUNTER_OUT_CREATE_CHILD_SA_REQ,
	/** sent CREATE_CHILD_SA responses */
	COUNTER_OUT_CREATE_CHILD_SA_RSP,
	/** received INFORMATIONAL requests */
	COUNTER_IN_INFORMATIONAL_REQ,
	/** received INFORMATIONAL responses */
	COUNTER_IN_INFORMATIONAL_RSP,
	/** sent INFORMATIONAL requests */
	COUNTER_OUT_INFORMATIONAL_REQ,
	/** sent INFORMATIONAL responses */
	COUNTER_OUT_INFORMATIONAL_RSP,
	/** number of counter types */
	COUNTER_MAX
};

/**
 * Query counter values for different IKE events.
 */
struct counters_query_t {

	/**
	 * Enumerate all connection names for which counters are currently recorded.
	 *
	 * @return				enumerator over names (char *)
	 */
	enumerator_t *(*get_names)(counters_query_t *this);

	/**
	 * Get a current global or connection-specific counter value.
	 *
	 * @param type			counter to query
	 * @param name			connection name to get counter for, NULL for global
	 * @param[out] value	counter value
	 * @return				TRUE if value found and returned
	 */
	bool (*get)(counters_query_t *this, counter_type_t type, char *name,
				uint64_t *value);

	/**
	 * Get all global or connection-specific counter values.
	 *
	 * @param name			connection name to get counters for, NULL for global
	 * @return				array of counters (has to be freed), NULL if named
	 *						connection is not found
	 */
	uint64_t *(*get_all)(counters_query_t *this, char *name);

	/**
	 * Reset all global or connection-specific counters.
	 *
	 * @param name			connection name to reset counters, NULL for global
	 */
	void (*reset)(counters_query_t *this, char *name);

	/**
	 * Reset counters for all connections, global counters are unaffected.
	 */
	void (*reset_all)(counters_query_t *this);
};

#endif /** COUNTERS_QUERY_H_ @}*/
