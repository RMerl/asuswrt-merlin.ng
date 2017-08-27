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

/**
 * @defgroup stroke_counter stroke_counter
 * @{ @ingroup stroke
 */

#ifndef STROKE_COUNTER_H_
#define STROKE_COUNTER_H_

#include <bus/listeners/listener.h>

typedef struct stroke_counter_t stroke_counter_t;
typedef enum stroke_counter_type_t stroke_counter_type_t;

enum stroke_counter_type_t {
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
 * Collection of counter values for different IKE events.
 */
struct stroke_counter_t {

	/**
	 * Implements listener_t.
	 */
	listener_t listener;

	/**
	 * Print counter values to an output stream.
	 *
	 * @param out		output stream to write to
	 * @param name		connection name to get counters for, NULL for global
	 */
	void (*print)(stroke_counter_t *this, FILE *out, char *name);

	/**
	 * Reset global or connection specific counters.
	 *
	 * @param name		name of connection counters to reset, NULL for global
	 */
	void (*reset)(stroke_counter_t *this, char *name);

	/**
	 * Destroy a stroke_counter_t.
	 */
	void (*destroy)(stroke_counter_t *this);
};

/**
 * Create a stroke_counter instance.
 */
stroke_counter_t *stroke_counter_create();

#endif /** STROKE_COUNTER_H_ @}*/
