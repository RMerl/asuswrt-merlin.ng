/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup eap_manager eap_manager
 * @{ @ingroup eap
 */

#ifndef EAP_MANAGER_H_
#define EAP_MANAGER_H_

#include <sa/eap/eap_method.h>

typedef struct eap_manager_t eap_manager_t;

/**
 * The EAP manager manages all EAP implementations and creates instances.
 *
 * A plugin registers it's implemented EAP method at the manager by
 * providing type and a contructor function. The manager then instanciates
 * eap_method_t instances through the provided constructor to handle
 * EAP authentication.
 */
struct eap_manager_t {

	/**
	 * Register a EAP method implementation.
	 *
	 * @param method		vendor specific method, if vendor != 0
	 * @param vendor		vendor ID, 0 for non-vendor (default) EAP methods
	 * @param role			EAP role of the registered method
	 * @param constructor	constructor function, returns an eap_method_t
	 */
	void (*add_method)(eap_manager_t *this, eap_type_t type, u_int32_t vendor,
					   eap_role_t role, eap_constructor_t constructor);

	/**
	 * Unregister a EAP method implementation using it's constructor.
	 *
	 * @param constructor	constructor function to remove, as added in add_method
	 */
	void (*remove_method)(eap_manager_t *this, eap_constructor_t constructor);

	/**
	 * Enumerate the registered EAP authentication methods for the given role.
	 *
	 * @note Only authentication types are enumerated (e.g. EAP-Identity is not
	 * even though it is registered as method with this manager).
	 *
	 * @param role			EAP role of methods to enumerate
	 * @return				enumerator over (eap_type_t type, u_int32_t vendor)
	 */
	enumerator_t* (*create_enumerator)(eap_manager_t *this, eap_role_t role);

	/**
	 * Create a new EAP method instance.
	 *
	 * @param type			type of the EAP method
	 * @param vendor		vendor ID, 0 for non-vendor (default) EAP methods
	 * @param role			role of EAP method, either EAP_SERVER or EAP_PEER
	 * @param server		identity of the server
	 * @param peer			identity of the peer (client)
	 * @return				EAP method instance, NULL if no constructor found
	 */
	eap_method_t* (*create_instance)(eap_manager_t *this, eap_type_t type,
									 u_int32_t vendor, eap_role_t role,
									 identification_t *server,
									 identification_t *peer);

	/**
	 * Destroy a eap_manager instance.
	 */
	void (*destroy)(eap_manager_t *this);
};

/**
 * Create a eap_manager instance.
 */
eap_manager_t *eap_manager_create();

#endif /** EAP_MANAGER_H_ @}*/
