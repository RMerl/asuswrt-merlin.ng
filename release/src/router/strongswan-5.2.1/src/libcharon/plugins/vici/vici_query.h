/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup vici_query vici_query
 * @{ @ingroup vici
 */

#include "vici_dispatcher.h"

#ifndef VICI_QUERY_H_
#define VICI_QUERY_H_

typedef struct vici_query_t vici_query_t;

/**
 * Query helper, provides various commands to query/list daemon info.
 */
struct vici_query_t {

	/**
	 * Destroy a vici_query_t.
	 */
	void (*destroy)(vici_query_t *this);
};

/**
 * Create a vici_query instance.
 *
 * @param dispatcher		dispatcher to receive requests from
 * @return					query handler
 */
vici_query_t *vici_query_create(vici_dispatcher_t *dispatcher);

#endif /** VICI_QUERY_H_ @}*/
