/*
 * Copyright (C) 2010-2014 Martin Willi
 * Copyright (C) 2010-2014 revosec AG
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
 * @defgroup forecast_listener forecast_listener
 * @{ @ingroup forecast
 */

#ifndef FORECAST_LISTENER_H_
#define FORECAST_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct forecast_listener_t forecast_listener_t;

/**
 * Listener to register the set of IPs we forward received multi/broadcasts to.
 */
struct forecast_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Create an enumerator over active tunnels.
	 *
	 * The enumerator enumerates over local or remote traffic selectors,
	 * associated firewall marks and if decasulated packets should get
	 * reinjected into other tunnels.
	 *
	 * @param local		TRUE to enumerate local, FALSE to enumerate remote TS
	 * @return			enumerator over (traffic_selector_t*, u_int, bool)
	 */
	enumerator_t* (*create_enumerator)(forecast_listener_t *this, bool local);

	/**
	 * Set the broadcast address of the LAN interface.
	 *
	 * @param bcast		broadcast address
	 */
	void (*set_broadcast)(forecast_listener_t *this, host_t *bcast);

	/**
	 * Destroy a forecast_listener_t.
	 */
	void (*destroy)(forecast_listener_t *this);
};

/**
 * Create a forecast_listener instance.
 */
forecast_listener_t *forecast_listener_create();

#endif /** FORECAST_LISTENER_H_ @}*/
