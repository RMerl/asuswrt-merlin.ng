/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup updown_handler updown_handler
 * @{ @ingroup updown
 */

#ifndef UPDOWN_HANDLER_H_
#define UPDOWN_HANDLER_H_

#include <attributes/attribute_handler.h>

typedef struct updown_handler_t updown_handler_t;

/**
 * Handler storing configuration attributes to pass to updown script.
 */
struct updown_handler_t {

	/**
	 * Implements the attribute_handler_t interface
	 */
	attribute_handler_t handler;

	/**
	 * Create an enumerator over received DNS servers.
	 *
	 * @param id		unique IKE_SA identifier to get attributes for
	 * @return			enumerator over host_t*
	 */
	enumerator_t* (*create_dns_enumerator)(updown_handler_t *this, u_int id);

	/**
	 * Destroy a updown_handler_t.
	 */
	void (*destroy)(updown_handler_t *this);
};

/**
 * Create a updown_handler instance.
 */
updown_handler_t *updown_handler_create();

#endif /** UPDOWN_HANDLER_H_ @}*/
