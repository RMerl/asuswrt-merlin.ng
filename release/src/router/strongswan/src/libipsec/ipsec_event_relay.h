/*
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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
 * @defgroup ipsec_event_relay ipsec_event_relay
 * @{ @ingroup libipsec
 */

#ifndef IPSEC_EVENT_RELAY_H_
#define IPSEC_EVENT_RELAY_H_

#include "ipsec_event_listener.h"

#include <library.h>

typedef struct ipsec_event_relay_t ipsec_event_relay_t;

/**
 *  Event relay manager.
 *
 *  Used to notify upper layers about changes
 */
struct ipsec_event_relay_t {

	/**
	 * Raise an expire event.
	 *
	 * @param protocol		protocol (e.g ESP) of the expired SA
	 * @param spi			SPI of the expired SA
	 * @param dst			destination address of expired SA
	 * @param hard			TRUE for a hard expire, FALSE otherwise
	 */
	void (*expire)(ipsec_event_relay_t *this, uint8_t protocol, uint32_t spi,
				   host_t *dst, bool hard);

	/**
	 * Register a listener to events raised by this manager
	 *
	 * @param listener		the listener to register
	 */
	void (*register_listener)(ipsec_event_relay_t *this,
							  ipsec_event_listener_t *listener);

	/**
	 * Unregister a listener
	 *
	 * @param listener		the listener to unregister
	 */
	void (*unregister_listener)(ipsec_event_relay_t *this,
								ipsec_event_listener_t *listener);

	/**
	 * Destroy an ipsec_event_relay_t
	 */
	void (*destroy)(ipsec_event_relay_t *this);

};

/**
 * Create an ipsec_event_relay_t instance
 *
 * @return			IPsec event relay instance
 */
ipsec_event_relay_t *ipsec_event_relay_create();

#endif /** IPSEC_EVENT_RELAY_H_ @}*/
