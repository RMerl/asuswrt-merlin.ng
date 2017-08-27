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
 * @defgroup whitelist_listener whitelist_listener
 * @{ @ingroup whitelist
 */

#ifndef WHITELIST_LISTENER_H_
#define WHITELIST_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct whitelist_listener_t whitelist_listener_t;

/**
 * Listener checking connecting peer against a whitelist.
 */
struct whitelist_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Add a peer identity to the whitelist.
	 *
	 * @param id		identity to whitelist
	 */
	void (*add)(whitelist_listener_t *this, identification_t *id);

	/**
	 * Remove a peer identity from the whitelist.
	 *
	 * @param id		identity to remove from whitelist
	 */
	void (*remove)(whitelist_listener_t *this, identification_t *id);

	/**
	 * Create an enumerator over whitelisted peer identities.
	 *
	 * The enumerator read-locks the whitelist, do not call add/remove while
	 * it is alive.
	 *
	 * @return			enumerator over identification_t*
	 */
	enumerator_t* (*create_enumerator)(whitelist_listener_t *this);

	/**
	 * Flush identities from whitelist matching id.
	 *
	 * @param id		id to match
	 */
	void (*flush)(whitelist_listener_t *this, identification_t *id);

	/**
	 * Enable/Disable whitelist checking.
	 *
	 * @param enable	TRUE to enable, FALSE to disable
	 */
	void (*set_active)(whitelist_listener_t *this, bool enable);

	/**
	 * Destroy a whitelist_listener_t.
	 */
	void (*destroy)(whitelist_listener_t *this);
};

/**
 * Create a whitelist_listener instance.
 */
whitelist_listener_t *whitelist_listener_create();

#endif /** WHITELIST_LISTENER_H_ @}*/
