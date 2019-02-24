/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2008 Philip Boetschi, Adrian Doerig
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
 * @defgroup peer_controller_server peer_controller
 * @{ @ingroup medsrv
 */

#ifndef PEER_CONTROLLER_H_
#define PEER_CONTROLLER_H_

#include <user.h>

#include <fast_controller.h>
#include <database/database.h>

typedef struct peer_controller_t peer_controller_t;

/**
 * Peer controller. Manages peers associated to a user.
 */
struct peer_controller_t {

	/**
	 * Implements controller_t interface.
	 */
	fast_controller_t controller;
};

/**
 * Create a peer_controller controller instance.
 */
fast_controller_t *peer_controller_create(user_t *user, database_t *db);

#endif /** PEER_CONTROLLER_H_ @}*/
