/*
 * Copyright (C) 2007 Tobias Brunner
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
 * @defgroup ike_me ike_me
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_ME_H_
#define IKE_ME_H_

typedef struct ike_me_t ike_me_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type TASK_IKE_ME, detects and handles IKE-ME extensions.
 *
 * This tasks handles the ME_MEDIATION Notify exchange to setup a mediation
 * connection, allows to initiate mediated connections using ME_CONNECT
 * exchanges and to request reflexive addresses from the mediation server using
 * ME_ENDPOINT notifies.
 *
 * @note This task has to be activated before the IKE_AUTH task, because that
 * task generates the IKE_SA_INIT message so that no more payloads can be added
 * to it afterwards.
 */
struct ike_me_t {
	/**
	 * Implements the task_t interface
	 */
	task_t task;

	/**
	 * Initiates a connection with another peer (i.e. sends a ME_CONNECT
	 * to the mediation server)
	 *
	 * @param peer_id			ID of the other peer (gets cloned)
	 */
	void (*connect)(ike_me_t *this, identification_t *peer_id);

	/**
	 * Responds to a ME_CONNECT from another peer (i.e. sends a ME_CONNECT
	 * to the mediation server)
	 *
	 * Data gets cloned.
	 *
	 * @param peer_id			ID of the other peer
	 * @param connect_id		the connect ID as provided by the initiator
	 */
	void (*respond)(ike_me_t *this, identification_t *peer_id,
					chunk_t connect_id);

	/**
	 * Sends a ME_CALLBACK to a peer that previously requested some other peer.
	 *
	 * @param peer_id			ID of the other peer (gets cloned)
	 */
	void (*callback)(ike_me_t *this, identification_t *peer_id);

	/**
	 * Relays data to another peer (i.e. sends a ME_CONNECT to the peer)
	 *
	 * Data gets cloned.
	 *
	 * @param requester			ID of the requesting peer
	 * @param connect_id		content of the ME_CONNECTID notify
	 * @param connect_key		content of the ME_CONNECTKEY notify
	 * @param endpoints			endpoints
	 * @param response			TRUE if this is a response
	 */
	void (*relay)(ike_me_t *this, identification_t *requester,
				  chunk_t connect_id, chunk_t connect_key,
				  linked_list_t *endpoints, bool response);
};

/**
 * Create a new ike_me task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task is initiated by us
 * @return				ike_me task to be handled by the task_manager
 */
ike_me_t *ike_me_create(ike_sa_t *ike_sa, bool initiator);

#endif /** IKE_ME_H_ @}*/
