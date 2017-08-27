/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup lookip_listener lookip_listener
 * @{ @ingroup lookip
 */

#ifndef LOOKIP_LISTENER_H_
#define LOOKIP_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct lookip_listener_t lookip_listener_t;

/**
 * Callback function to query virtual IP entries
 *
 * @param user		user supplied pointer
 * @param up		TRUE if tunnels established, FALSE if closed
 * @param vip		virtual IP of remote peer
 * @param other		peer external IP
 * @param id		peer identity
 * @param name		associated connection name
 * @param unique_id	unique IKE_SA identifier
 * @return			TRUE to receive more results, FALSE to cancel
 */
typedef bool (*lookip_callback_t)(void *user, bool up, host_t *vip,
								  host_t *other, identification_t *id,
								  char *name, u_int unique_id);

/**
 * Listener collecting virtual IPs.
 */
struct lookip_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Perform a lookup for a given virtual IP, invoke callback for matches.
	 *
	 * The "up" parameter is always TRUE when the callback is invoked using
	 * lookup().
	 *
	 * @param vip		virtual IP to look up, NULL to get all entries
	 * @param cb		callback function to invoke
	 * @param user		user data to pass to callback function
	 * @return			number of matches
	 */
	int (*lookup)(lookip_listener_t *this, host_t *vip,
				  lookip_callback_t cb, void *user);

	/**
	 * Register a listener function that gets notified about virtual IP changes.
	 *
	 * @param cb		callback function to invoke
	 * @param user		user data to pass to callback function
	 */
	void (*add_listener)(lookip_listener_t *this,
						 lookip_callback_t cb, void *user);

	/**
	 * Unregister a listener by the user data.
	 *
	 * @param user		user data, as passed during add_listener()
	 */
	void (*remove_listener)(lookip_listener_t *this, void *user);

	/**
	 * Destroy a lookip_listener_t.
	 */
	void (*destroy)(lookip_listener_t *this);
};

/**
 * Create a lookip_listener instance.
 */
lookip_listener_t *lookip_listener_create();

#endif /** LOOKIP_LISTENER_H_ @}*/
