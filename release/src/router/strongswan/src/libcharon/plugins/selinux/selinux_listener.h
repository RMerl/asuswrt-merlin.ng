/*
 * Copyright (C) 2022 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup selinux_listener selinux_listener
 * @{ @ingroup selinux
 */

#ifndef SELINUX_LISTENER_H_
#define SELINUX_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct selinux_listener_t selinux_listener_t;

/**
 * Listener to manage trap policies for generic SELinux labels.
 */
struct selinux_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Destroy a selinux_listener_t.
	 */
	void (*destroy)(selinux_listener_t *this);
};

/**
 * Create a listener instance.
 */
selinux_listener_t *selinux_listener_create();

#endif /** SELINUX_LISTENER_H_ @}*/
