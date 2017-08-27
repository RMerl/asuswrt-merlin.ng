/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2008 Philip Boetschi, Adrian Doerig
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
 * @defgroup user_controller_server user_controller
 * @{ @ingroup medsrv
 */

#ifndef USER_CONTROLLER_H_
#define USER_CONTROLLER_H_

#include <user.h>

#include <fast_controller.h>
#include <database/database.h>

typedef struct user_controller_t user_controller_t;

/**
 * User controller. Register, Login and user management.
 */
struct user_controller_t {

	/**
	 * Implements controller_t interface.
	 */
	fast_controller_t controller;
};

/**
 * Create a user_controller controller instance.
 */
fast_controller_t *user_controller_create(user_t *user, database_t *db);

#endif /** USER_CONTROLLER_H_ @}*/
