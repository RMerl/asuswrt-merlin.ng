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
 * @defgroup medsrv medsrv
 *
 * @defgroup user user
 * @{ @ingroup medsrv
 */

#ifndef USER_H_
#define USER_H_

#include <fast_context.h>
#include <library.h>

typedef struct user_t user_t;

/**
 * Per session context. Contains user user state and data.
 */
struct user_t {

	/**
	 * implements context_t interface
	 */
	fast_context_t context;

	/**
	 * Set the user ID of the logged in user.
	 */
	void (*set_user)(user_t *this, u_int id);

	/**
	 * Get the user ID of the logged in user.
	 */
	u_int (*get_user)(user_t *this);
};

/**
 * Create a user instance.
 */
user_t *user_create(void *param);

#endif /** USER_H_ @} */
