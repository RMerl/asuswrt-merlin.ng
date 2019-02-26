/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup connmark_listener connmark_listener
 * @{ @ingroup connmark
 */

#ifndef CONNMARK_LISTENER_H_
#define CONNMARK_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct connmark_listener_t connmark_listener_t;

/**
 * Listener to install Netfilter rules
 */
struct connmark_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Destroy a connmark_listener_t.
	 */
	void (*destroy)(connmark_listener_t *this);
};

/**
 * Create a connmark_listener instance.
 */
connmark_listener_t *connmark_listener_create();

#endif /** CONNMARK_LISTENER_H_ @}*/
