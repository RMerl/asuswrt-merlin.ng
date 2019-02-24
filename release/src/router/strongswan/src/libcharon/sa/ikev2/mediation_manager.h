/*
 * Copyright (C) 2007 Tobias Brunner
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
 * @defgroup mediation_manager mediation_manager
 * @{ @ingroup ikev2
 */

#ifndef MEDIATION_MANAGER_H_
#define MEDIATION_MANAGER_H_

typedef struct mediation_manager_t mediation_manager_t;

#include <sa/ike_sa_id.h>
#include <utils/identification.h>

/**
 * The mediation manager is responsible for managing currently online
 * peers and registered requests for offline peers on the mediation server.
 */
struct mediation_manager_t {

	/**
	 * Remove the IKE_SA of a peer.
	 *
	 * @param ike_sa_id			the IKE_SA ID of the peer's SA
	 */
	void (*remove) (mediation_manager_t* this, ike_sa_id_t *ike_sa_id);

	/**
	 * Update the ike_sa_id that is assigned to a peer's ID. If the peer
	 * is new, it gets a new record assigned.
	 *
	 * @param peer_id			the peer's ID
	 * @param ike_sa_id			the IKE_SA ID of the peer's SA
	 */
	void (*update_sa_id) (mediation_manager_t* this, identification_t *peer_id,
						  ike_sa_id_t *ike_sa_id);

	/**
	 * Checks if a specific peer is online.
	 *
	 * @param peer_id			the peer's ID
	 * @returns
	 *							- IKE_SA ID of the peer's SA.
	 *							- NULL, if the peer is not online.
	 */
	ike_sa_id_t* (*check) (mediation_manager_t* this,
						   identification_t *peer_id);

	/**
	 * Checks if a specific peer is online and registers the requesting
	 * peer if it is not.
	 *
	 * @param peer_id			the peer's ID
	 * @param requester			the requesters ID
	 * @returns
	 *							- IKE_SA ID of the peer's SA.
	 *							- NULL, if the peer is not online.
	 */
	ike_sa_id_t* (*check_and_register) (mediation_manager_t* this,
										identification_t *peer_id,
										identification_t *requester);

	/**
	 * Destroys the manager with all data.
	 */
	void (*destroy) (mediation_manager_t *this);
};

/**
 * Create a manager.
 *
 * @returns	mediation_manager_t object
 */
mediation_manager_t *mediation_manager_create(void);

#endif /** MEDIATION_MANAGER_H_ @}*/
