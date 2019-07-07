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
 * @defgroup xpc_logger xpc_logger
 * @{ @ingroup xpc
 */

#ifndef XPC_LOGGER_H_
#define XPC_LOGGER_H_

#include <xpc/xpc.h>

#include <daemon.h>

typedef struct xpc_logger_t xpc_logger_t;

/**
 * Connection specific logger over XPC.
 */
struct xpc_logger_t {

	/**
	 * Implements logger_t.
	 */
	logger_t logger;

	/**
	 * Set the IKE_SA unique identifier this logger logs for.
	 *
	 * @param ike_sa		IKE_SA unique identifier
	 */
	void (*set_ike_sa)(xpc_logger_t *this, uint32_t ike_sa);

	/**
	 * Destroy a xpc_logger_t.
	 */
	void (*destroy)(xpc_logger_t *this);
};

/**
 * Create a xpc_logger instance.
 *
 * @param conn		XPC connection to send logging events to
 * @return			XPC logger
 */
xpc_logger_t *xpc_logger_create(xpc_connection_t conn);

#endif /** XPC_LOGGER_H_ @}*/
