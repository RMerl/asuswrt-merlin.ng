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
 * @defgroup load_tester_listener_t load_tester_listener
 * @{ @ingroup load_tester
 */

#ifndef LOAD_TESTER_LISTENER_H_
#define LOAD_TESTER_LISTENER_H_

#include <bus/bus.h>

#include "load_tester_config.h"

typedef struct load_tester_listener_t load_tester_listener_t;

/**
 * Provide hard-coded credentials for load testing.
 */
struct load_tester_listener_t {

	/**
	 * Implements listener set interface.
	 */
	listener_t listener;

	/**
	 * Get the number of established IKE_SAs.
	 *
	 * @return			number of SAs currently established
	 */
	u_int (*get_established)(load_tester_listener_t *this);

	/**
	 * Destroy the backend.
	 */
	void (*destroy)(load_tester_listener_t *this);
};

/**
 * Create a listener to handle special events during load test
 *
 * @param shutdown_on	shut down the daemon after this many SAs are established
 * @param config		configuration backend
 * @return				listener
 */
load_tester_listener_t *load_tester_listener_create(u_int shutdown_on,
													load_tester_config_t *config);

#endif /** LOAD_TESTER_LISTENER_H_ @}*/
