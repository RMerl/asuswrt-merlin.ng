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
 * @defgroup auth_filter_server auth_filter
 * @{ @ingroup medsrv
 */

#ifndef AUTH_FILTER_H_
#define AUTH_FILTER_H_

#include <library.h>
#include <fast_filter.h>

#include "user.h"

typedef struct auth_filter_t auth_filter_t;

/**
 * Authentication/Authorization filter.
 */
struct auth_filter_t {

	/**
	 * Implements filter_t interface.
	 */
	fast_filter_t filter;
};

/**
 * Create a auth_filter instance.
 */
fast_filter_t *auth_filter_create(user_t *user, database_t *db);

#endif /** AUTH_FILTER_H_  @}*/
