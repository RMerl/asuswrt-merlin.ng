/*
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup ikesa_controller ikesa_controller
 * @{ @ingroup manager_controller
 */

#ifndef IKESA_CONTROLLER_H_
#define IKESA_CONTROLLER_H_

#include <fast_controller.h>

typedef struct ikesa_controller_t ikesa_controller_t;

/**
 * Status controller.
 */
struct ikesa_controller_t {

	/**
	 * Implements controller_t interface.
	 */
	fast_controller_t controller;
};

/**
 * Create a ikesa_controller controller instance.
 */
fast_controller_t *ikesa_controller_create(fast_context_t *context, void *param);

#endif /** IKESA_CONTROLLER_H_ @}*/
