/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup eap_inner_method eap_inner_method
 * @{ @ingroup eap
 */

#ifndef EAP_INNER_METHOD_H_
#define EAP_INNER_METHOD_H_

typedef struct eap_inner_method_t eap_inner_method_t;

#include <library.h>

#include "eap_method.h"

/**
 * Interface of a weak inner EAP method like EAP-TNC or PT-EAP
 * that must be encapsulated in a strong TLS-based EAP method 
 */
struct eap_inner_method_t {

	/*
	 * Public EAP method interface
	 */
	eap_method_t eap_method;

	/*
	 * Get type of outer EAP authentication method
	 *
	 * @return			outer EAP authentication type
	 */
	eap_type_t (*get_auth_type)(eap_inner_method_t *this); 

	/*
	 * Set type of outer EAP Client/Server authentication
	 *
	 * @param type		outer EAP authentication type
	 */
	void (*set_auth_type)(eap_inner_method_t *this, eap_type_t type); 

};

#endif /** EAP_INNER_METHOD_H_ @}*/
