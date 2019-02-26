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
 * @defgroup attribute_handler attribute_handler
 * @{ @ingroup attributes
 */

#ifndef ATTRIBUTE_HANDLER_H_
#define ATTRIBUTE_HANDLER_H_

typedef struct attribute_handler_t attribute_handler_t;

#include <sa/ike_sa.h>
#include <utils/chunk.h>
#include <collections/linked_list.h>

#include "attributes.h"

/**
 * Interface to handle configuration payload attributes.
 */
struct attribute_handler_t {

	/**
	 * Handle a configuration attribute.
	 *
	 * After receiving a configuration attriubte, it is passed to each
	 * attribute handler until it is handled.
	 *
	 * @param ike_sa	IKE_SA under which attribute is received
	 * @param type		type of configuration attribute to handle
	 * @param data		associated attribute data
	 * @return			TRUE if attribute handled
	 */
	bool (*handle)(attribute_handler_t *this, ike_sa_t *ike_sa,
				   configuration_attribute_type_t type, chunk_t data);

	/**
	 * Release an attribute handled during handle().
	 *
	 * A handler that handle()d an attribute gets a call to release() when the
	 * connection gets closed. Depending on the implementation, this is required
	 * to remove the attribute.
	 *
	 * @param ike_sa	IKE_SA which releases attribute
	 * @param type		type of configuration attribute to release
	 * @param data		associated attribute data
	 */
	void (*release)(attribute_handler_t *this, ike_sa_t *ike_sa,
					configuration_attribute_type_t type, chunk_t data);

	/**
	 * Enumerate attributes to request from a server.
	 *
	 * @param ike_sa		IKE_SA to request attributes for
	 * @param vips			list of virtual IPs (host_t*) we are requesting
	 * @return				enumerator (configuration_attribute_type_t, chunk_t)
	 */
	enumerator_t* (*create_attribute_enumerator)(attribute_handler_t *this,
										ike_sa_t *ike_sa, linked_list_t *vips);
};

#endif /** ATTRIBUTE_HANDLER_H_ @}*/
