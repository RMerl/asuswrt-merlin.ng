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
 * @defgroup unity_handler unity_handler
 * @{ @ingroup unity
 */

#ifndef UNITY_HANDLER_H_
#define UNITY_HANDLER_H_

#include <sa/ike_sa_id.h>
#include <attributes/attribute_handler.h>

typedef struct unity_handler_t unity_handler_t;

/**
 * Cisco Unity attribute handling.
 */
struct unity_handler_t {

	/**
	 * Implements attribute_handler_t.
	 */
	attribute_handler_t handler;

	/**
	 * Create an enumerator over Split-Include attributes received for an IKE_SA.
	 *
	 * @param id			IKE_SA ID to get Split-Includes for
	 * @return				enumerator over traffic_selector_t*
	 */
	enumerator_t* (*create_include_enumerator)(unity_handler_t *this,
											   ike_sa_id_t *id);

	/**
	 * Destroy a unity_handler_t.
	 */
	void (*destroy)(unity_handler_t *this);
};

/**
 * Create a unity_handler instance.
 */
unity_handler_t *unity_handler_create();

#endif /** UNITY_HANDLER_H_ @}*/
