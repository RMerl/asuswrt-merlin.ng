/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup eap_sim_server eap_sim_server
 * @{ @ingroup eap_sim
 */

#ifndef EAP_SIM_SERVER_H_
#define EAP_SIM_SERVER_H_

#include <sa/eap/eap_method.h>

typedef struct eap_sim_server_t eap_sim_server_t;

/**
 * EAP-SIM server implementation.
 */
struct eap_sim_server_t {

	/**
	 * Implemented eap_method_t interface.
	 */
	eap_method_t interface;

	/**
	 * Destroy a eap_sim_server_t.
	 */
	void (*destroy)(eap_sim_server_t *this);
};

/**
 * Creates the EAP method EAP-SIM acting as server.
 *
 * @param server	ID of the EAP server
 * @param peer		ID of the EAP peer
 * @return			eap_sim_t object
 */
eap_sim_server_t *eap_sim_server_create(identification_t *server,
										identification_t *peer);

#endif /** EAP_SIM_SERVER_H_ @}*/
