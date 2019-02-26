/*
 * Copyright (C) 2012 Reto Guadagnini
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
 * @defgroup unbound_response unbound_response
 * @{ @ingroup unbound_p
 */

#ifndef UNBOUND_RESPONSE_H_
#define UNBOUND_RESPONSE_H_

#include <resolver/resolver_response.h>
#include <unbound.h>

typedef struct unbound_response_t unbound_response_t;

/**
 * Implementation of the resolver_response interface using libunbound.
 *
 */
struct unbound_response_t {

	/**
	 * Implements the resolver_response interface
	 */
	resolver_response_t interface;
};

/**
 * Create an unbound_response instance from a response of the unbound library.
 *
 * @param	response	a response of the unbound library
 * @return				an unbound_response conforming to the resolver_response
 * 						interface, or NULL on failure
 */
unbound_response_t *unbound_response_create_frm_libub_response(
													struct ub_result *response);

#endif /** UNBOUND_RESPONSE_H_ @}*/
