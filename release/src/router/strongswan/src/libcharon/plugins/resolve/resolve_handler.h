/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup resolve_handler resolve_handler
 * @{ @ingroup resolve
 */

#ifndef RESOLVE_HANDLER_H_
#define RESOLVE_HANDLER_H_

#include <attributes/attribute_handler.h>

typedef struct resolve_handler_t resolve_handler_t;

/**
 * Handle DNS configuration attributes by mangling a resolv.conf file.
 */
struct resolve_handler_t {

	/**
	 * Implements the attribute_handler_t interface
	 */
	attribute_handler_t handler;

	/**
	 * Destroy a resolve_handler_t.
	 */
	void (*destroy)(resolve_handler_t *this);
};

/**
 * Create a resolve_handler instance.
 */
resolve_handler_t *resolve_handler_create();

#endif /** RESOLVE_HANDLER_H_ @}*/
