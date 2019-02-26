/*
 * Copyright (C) 2016 Tobias Brunner
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
 * @defgroup bypass_lan_listener bypass_lan_listener
 * @{ @ingroup bypass_lan
 */

#ifndef BYPASS_LAN_LISTENER_H_
#define BYPASS_LAN_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct bypass_lan_listener_t bypass_lan_listener_t;

/**
 * Listener to install bypass policies
 */
struct bypass_lan_listener_t {

	/**
	 * Implements kernel_listener_t interface.
	 */
	kernel_listener_t listener;

	/**
	 * Reload ignored/used interface names from config.
	 */
	void (*reload_interfaces)(bypass_lan_listener_t *this);

	/**
	 * Destroy a bypass_lan_listener_t.
	 */
	void (*destroy)(bypass_lan_listener_t *this);
};

/**
 * Create a bypass_lan_listener instance.
 */
bypass_lan_listener_t *bypass_lan_listener_create();

#endif /** BYPASS_LAN_LISTENER_H_ @}*/
