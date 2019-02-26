/*
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup fast_session fast_session
 * @{ @ingroup libfast
 */

#ifndef FAST_SESSION_H_
#define FAST_SESSION_H_

#include "fast_request.h"
#include "fast_controller.h"
#include "fast_filter.h"

typedef struct fast_session_t fast_session_t;

/**
 * Session handling class, instantiated for each user session.
 */
struct fast_session_t {

	/**
	 * Get the session ID of the session.
	 *
	 * @return				session ID
	 */
	char* (*get_sid)(fast_session_t *this);

	/**
	 * Add a controller instance to the session.
	 *
	 * @param controller	controller to add
	 */
	void (*add_controller)(fast_session_t *this, fast_controller_t *controller);

	/**
	 * Add a filter instance to the session.
	 *
	 * @param filter		filter to add
	 */
	void (*add_filter)(fast_session_t *this, fast_filter_t *filter);

	/**
	 * Process a request in this session.
	 *
	 * @param request		request to process
	 */
	void (*process)(fast_session_t *this, fast_request_t *request);

	/**
	 * Destroy the fast_session_t.
	 */
	void (*destroy) (fast_session_t *this);
};

/**
 * Create a session new session.
 *
 * @param context		user defined session context instance
 * @return				client session, NULL on error
 */
fast_session_t *fast_session_create(fast_context_t *context);

#endif /** SESSION_H_ @}*/
