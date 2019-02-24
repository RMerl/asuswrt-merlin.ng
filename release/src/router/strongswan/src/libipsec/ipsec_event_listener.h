/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup ipsec_event_listener ipsec_event_listener
 * @{ @ingroup libipsec
 */

#ifndef IPSEC_EVENT_LISTENER_H_
#define IPSEC_EVENT_LISTENER_H_

typedef struct ipsec_event_listener_t ipsec_event_listener_t;

#include <library.h>

/**
 * Listener interface for IPsec events
 *
 * All methods are optional.
 */
struct ipsec_event_listener_t {

	/**
	 * Called when the lifetime of an IPsec SA expired
	 *
	 * @param protocol		protocol of the expired SA
	 * @param spi			spi of the expired SA
	 * @param dst			destination address of expired SA
	 * @param hard			TRUE if this is a hard expire, FALSE otherwise
	 */
	void (*expire)(uint8_t protocol, uint32_t spi, host_t *dst, bool hard);
};

#endif /** IPSEC_EVENT_LISTENER_H_ @}*/
