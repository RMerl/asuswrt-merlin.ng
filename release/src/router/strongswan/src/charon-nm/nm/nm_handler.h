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
 * @defgroup nm_handler nm_handler
 * @{ @ingroup nm
 */

#ifndef NM_HANDLER_H_
#define NM_HANDLER_H_

#include <attributes/attribute_handler.h>

typedef struct nm_handler_t nm_handler_t;

/**
 * Handles DNS/NBNS attributes to pass to NM.
 */
struct nm_handler_t {

	/**
	 * Implements attribute handler interface
	 */
	attribute_handler_t handler;

	/**
	 * Create an enumerator over received attributes of a given kind.
	 *
	 * @param type		type of attributes to enumerate
	 * @return			enumerator over attribute data (chunk_t*)
	 */
	enumerator_t* (*create_enumerator)(nm_handler_t *this,
									   configuration_attribute_type_t type);

	/**
	 * Reset state, flush all received attributes.
	 */
	void (*reset)(nm_handler_t *this);

	/**
	 * Destroy a nm_handler_t.
	 */
	void (*destroy)(nm_handler_t *this);
};

/**
 * Create a nm_handler instance.
 */
nm_handler_t *nm_handler_create();

#endif /** NM_HANDLER_H_ @}*/
