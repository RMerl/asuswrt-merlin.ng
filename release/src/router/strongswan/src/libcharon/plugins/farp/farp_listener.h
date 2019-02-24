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
 * @defgroup farp_listener farp_listener
 * @{ @ingroup farp
 */

#ifndef FARP_LISTENER_H_
#define FARP_LISTENER_H_

#include <networking/host.h>
#include <bus/listeners/listener.h>

typedef struct farp_listener_t farp_listener_t;

/**
 * Listener to register the set of IPs we spoof ARP responses for.
 */
struct farp_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Check if we have a tunnel between two IP addresses.
	 *
	 * @param local		local IP
	 * @param remote	remote IP
	 * @return			TRUE if a tunnel is active
	 */
	bool (*has_tunnel)(farp_listener_t *this, host_t *local, host_t *remote);

	/**
	 * Destroy a farp_listener_t.
	 */
	void (*destroy)(farp_listener_t *this);
};

/**
 * Create a farp_listener instance.
 */
farp_listener_t *farp_listener_create();

#endif /** FARP_LISTENER_H_ @}*/
