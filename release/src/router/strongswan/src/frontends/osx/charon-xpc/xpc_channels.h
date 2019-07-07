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
 * @defgroup xpc charon-xpc
 *
 * @defgroup xpc_channels xpc_channels
 * @{ @ingroup xpc
 */

#ifndef XPC_CHANNELS_H_
#define XPC_CHANNELS_H_

#include <xpc/xpc.h>

#include <bus/bus.h>

typedef struct xpc_channels_t xpc_channels_t;

/**
 * XPC to App channel management.
 */
struct xpc_channels_t {

	/**
	 * Implements listener_t.
	 */
	listener_t listener;

	/**
	 * Associate an IKE_SA unique identifier to an XPC connection.
	 *
	 * @param conn			XPC connection to channel
	 * @param ike_sa		IKE_SA unique identifier to associate to connection
	 */
	void (*add)(xpc_channels_t *this, xpc_connection_t conn, uint32_t ike_sa);

	/**
	 * Destroy a xpc_channels_t.
	 */
	void (*destroy)(xpc_channels_t *this);
};

/**
 * Create a xpc_channels instance.
 */
xpc_channels_t *xpc_channels_create();

#endif /** XPC_CHANNELS_H_ @}*/
