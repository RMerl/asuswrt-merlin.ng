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
 * @defgroup fast_controller fast_controller
 * @{ @ingroup libfast
 */

#ifndef FAST_CONTROLLER_H_
#define FAST_CONTROLLER_H_

#include "fast_request.h"
#include "fast_context.h"

typedef struct fast_controller_t fast_controller_t;

/**
 * Constructor function for a controller.
 *
 * @param context		session specific context, implements context_t
 * @param param			user supplied param, as registered to the dispatcher
 */
typedef fast_controller_t *(*fast_controller_constructor_t)(
										fast_context_t* context, void *param);

/**
 * Controller interface, to be implemented by users controllers.
 *
 * Controller instances get created per session, so each session has an
 * associated set of private controller instances.
 * The controller handle function is called for each incoming request.
 */
struct fast_controller_t {

	/**
	 * Get the name of the controller.
	 *
	 * @return				name of the controller
	 */
	char* (*get_name)(fast_controller_t *this);

	/**
	 * Handle a HTTP request for that controller.
	 *
	 * Request URLs are parsed in the form
	 * controller_name/p1/p2/p3/p4/p5 with a maximum of 5 parameters. Each
	 * parameter not found in the request URL is set to NULL.
	 *
	 * @param request		HTTP request
	 * @param p1			first parameter
	 * @param p2			second parameter
	 * @param p3			third parameter
	 * @param p4			forth parameter
	 * @param p5			fifth parameter
	 * @return
	 */
	void (*handle)(fast_controller_t *this, fast_request_t *request,
				   char *p1, char *p2, char *p3, char *p4, char *p5);

	/**
	 * Destroy the controller instance.
	 */
	void (*destroy) (fast_controller_t *this);
};

#endif /** FAST_CONTROLLER_H_ @}*/
