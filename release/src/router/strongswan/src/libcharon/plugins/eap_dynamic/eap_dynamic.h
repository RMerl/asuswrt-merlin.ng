/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup eap_dynamic_i eap_dynamic
 * @{ @ingroup eap_dynamic
 */

#ifndef EAP_DYNAMIC_H_
#define EAP_DYNAMIC_H_

typedef struct eap_dynamic_t eap_dynamic_t;

#include <sa/eap/eap_method.h>

/**
 * Implementation of the eap_method_t interface for a virtual EAP method that
 * proxies other EAP methods and supports the selection of the actual method
 * by the client.
 */
struct eap_dynamic_t {

	/**
	 * Implemented eap_method_t interface
	 */
	eap_method_t interface;
};

/**
 * Create a dynamic EAP proxy serving any supported real method which is also
 * supported (or selected) by the client.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP client
 * @return			eap_dynamic_t object
 */
eap_dynamic_t *eap_dynamic_create(identification_t *server,
								  identification_t *peer);

#endif /** EAP_DYNAMIC_H_ @}*/
