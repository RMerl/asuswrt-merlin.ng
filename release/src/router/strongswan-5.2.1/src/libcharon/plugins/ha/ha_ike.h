/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup ha_ike ha_ike
 * @{ @ingroup ha
 */

#ifndef HA_IKE_H_
#define HA_IKE_H_

typedef struct ha_ike_t ha_ike_t;

#include "ha_socket.h"
#include "ha_tunnel.h"
#include "ha_segments.h"
#include "ha_cache.h"

#include <daemon.h>

/**
 * Listener to synchronize IKE_SAs.
 */
struct ha_ike_t {

	/**
	 * Implements bus listener interface.
	 */
	listener_t listener;

	/**
	 * Destroy a ha_ike_t.
	 */
	void (*destroy)(ha_ike_t *this);
};

/**
 * Create a ha_ike instance.
 *
 * @param socket		socket to use for sending synchronization messages
 * @param tunnel		tunnel securing sync messages, if any
 * @param cache			message cache
 * @return				IKE listener
 */
ha_ike_t *ha_ike_create(ha_socket_t *socket, ha_tunnel_t *tunnel,
						ha_cache_t *cache);

#endif /** HA_IKE_ @}*/
