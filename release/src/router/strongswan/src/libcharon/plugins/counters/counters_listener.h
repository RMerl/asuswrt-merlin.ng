/*
 * Copyright (C) 2017 Tobias Brunner
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

/**
 * @defgroup counters_listener counters_listener
 * @{ @ingroup counters
 */

#ifndef COUNTERS_LISTENER_H_
#define COUNTERS_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct counters_listener_t counters_listener_t;

/**
 * Collect counter values for different IKE events.
 */
struct counters_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Destroy a counters_listener_t.
	 */
	void (*destroy)(counters_listener_t *this);
};

/**
 * Create a counters_listener_t instance.
 */
counters_listener_t *counters_listener_create();

#endif /** COUNTERS_LISTENER_H_ @}*/
