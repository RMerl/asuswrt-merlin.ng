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
 * @defgroup xpc_dispatch xpc_dispatch
 * @{ @ingroup xpc
 */

#ifndef XPC_DISPATCH_H_
#define XPC_DISPATCH_H_

typedef struct xpc_dispatch_t xpc_dispatch_t;

/**
 * XPC dispatcher to control the daemon.
 */
struct xpc_dispatch_t {

	/**
	 * Destroy a xpc_dispatch_t.
	 */
	void (*destroy)(xpc_dispatch_t *this);
};

/**
 * Create a xpc_dispatch instance.
 */
xpc_dispatch_t *xpc_dispatch_create();

#endif /** XPC_DISPATCH_H_ @}*/
