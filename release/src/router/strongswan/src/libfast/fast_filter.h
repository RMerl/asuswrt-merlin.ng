/*
 * Copyright (C) 2008 Martin Willi
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

/*
 * @defgroup fast_filter fast_filter
 * @{ @ingroup libfast
 */

#ifndef FAST_FILTER_H_
#define FAST_FILTER_H_

#include "fast_request.h"
#include "fast_context.h"
#include "fast_controller.h"

typedef struct fast_filter_t fast_filter_t;

/**
 * Constructor function for a filter
 *
 * @param context		session specific context
 * @param param			user supplied param
 */
typedef fast_filter_t *(*fast_filter_constructor_t)(fast_context_t* context,
													void *param);

/**
 * Filter interface, to be implemented by users filters.
 */
struct fast_filter_t {

	/**
	 * Called before the controller handles the request.
	 *
	 * @param request		HTTP request
	 * @param p1			first parameter
	 * @param p2			second parameter
	 * @param p3			third parameter
	 * @param p4			forth parameter
	 * @param p5			fifth parameter
	 * @return				TRUE to continue request handling
	 */
	bool (*run)(fast_filter_t *this, fast_request_t *request,
				char *p0, char *p1, char *p2, char *p3, char *p4, char *p5);

	/**
	 * Destroy the filter instance.
	 */
	void (*destroy) (fast_filter_t *this);
};

#endif /* FAST_FILTER_H_ @} */
