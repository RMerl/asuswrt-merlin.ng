/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup whitelist_control whitelist_control
 * @{ @ingroup whitelist
 */

#ifndef WHITELIST_CONTROL_H_
#define WHITELIST_CONTROL_H_

#include "whitelist_listener.h"

typedef struct whitelist_control_t whitelist_control_t;

/**
 * Whitelist UNIX control socket.
 */
struct whitelist_control_t {

	/**
	 * Destroy a whitelist_control_t.
	 */
	void (*destroy)(whitelist_control_t *this);
};

/**
 * Create a whitelist_control instance.
 */
whitelist_control_t *whitelist_control_create(whitelist_listener_t *listener);

#endif /** WHITELIST_CONTROL_H_ @}*/
