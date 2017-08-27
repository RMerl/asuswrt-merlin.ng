/*
 * Copyright (C) 2007-2008 Tobias Brunner
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
 * @defgroup connect_manager connect_manager
 * @{ @ingroup ikev2
 */

#ifndef CONNECT_MANAGER_H_
#define CONNECT_MANAGER_H_

typedef struct connect_manager_t connect_manager_t;

#include <encoding/message.h>
#include <sa/ike_sa_id.h>
#include <utils/identification.h>

/**
 * The connection manager is responsible for establishing a direct
 * connection with another peer.
 */
struct connect_manager_t {

	/**
	 * Checks if a there is already a mediated connection registered
	 * between two peers.
	 *
	 * @param id			my id
	 * @param peer_id		the other peer's id
	 * @param mediated_sa	the IKE_SA ID of the mediated connection
	 * @returns
	 *						- TRUE, if a mediated connection is registered
	 *						- FALSE, otherwise
	 */
	bool (*check_and_register) (connect_manager_t *this, identification_t *id,
								identification_t *peer_id,
								ike_sa_id_t *mediated_sa);

	/**
	 * Checks if there are waiting connections with a specific peer.
	 * If so, reinitiate them.
	 *
	 * @param id			my id
	 * @param peer_id		the other peer's id
	 */
	void (*check_and_initiate) (connect_manager_t *this,
								ike_sa_id_t *mediation_sa, identification_t *id,
								identification_t *peer_id);

	/**
	 * Creates a checklist and sets the initiator's data.
	 *
	 * @param initiator		ID of the initiator
	 * @param responder		ID of the responder
	 * @param connect_id	the connect ID provided by the initiator
	 * @param key			the initiator's key
	 * @param endpoints		the initiator's endpoints
	 * @param is_initiator	TRUE, if the caller of this method is the initiator
	 * @returns				SUCCESS
	 */
	status_t (*set_initiator_data) (connect_manager_t *this,
									identification_t *initiator,
									identification_t *responder,
									chunk_t connect_id, chunk_t key,
									linked_list_t *endpoints,
									bool is_initiator);

	/**
	 * Updates a checklist and sets the responder's data. The checklist's
	 * state is advanced to WAITING which means that checks will be sent.
	 *
	 * @param connect_id	the connect ID
	 * @param chunk_t		the responder's key
	 * @param endpoints		the responder's endpoints
	 * @returns
	 *						- NOT_FOUND, if the checklist has not been found
	 *						- SUCCESS, otherwise
	 */
	status_t (*set_responder_data) (connect_manager_t *this,
									chunk_t connect_id, chunk_t key,
									linked_list_t *endpoints);

	/**
	 * Stops checks for a checklist. Called after the responder received an
	 * IKE_SA_INIT request which contains a ME_CONNECTID payload.
	 *
	 * @param connect_id	the connect ID
	 * @returns
	 *						- NOT_FOUND, if the checklist has not been found
	 *						- SUCCESS, otherwise
	 */
	status_t (*stop_checks) (connect_manager_t *this, chunk_t connect_id);

	/**
	 * Processes a connectivity check
	 *
	 * @param message		the received message
	 */
	void (*process_check) (connect_manager_t *this, message_t *message);

	/**
	 * Destroys the manager with all data.
	 */
	void (*destroy) (connect_manager_t *this);
};

/**
 * Create a manager.
 *
 * @returns	connect_manager_t object
 */
connect_manager_t *connect_manager_create(void);

#endif /** CONNECT_MANAGER_H_ @}*/
