/*
 * Copyright (C) 2016 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup child_rekey child_rekey
 * @{ @ingroup tasks_v2
 */

#ifndef CHILD_REKEY_H_
#define CHILD_REKEY_H_

typedef struct child_rekey_t child_rekey_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/child_sa.h>
#include <sa/task.h>

/**
 * Task of type TASK_CHILD_REKEY, rekey an established CHILD_SA.
 */
struct child_rekey_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;

	/**
	 * Check if the given SA is the redundant CHILD_SA created during a rekey
	 * collision.
	 *
	 * This is called if the other peer deletes the redundant SA before we were
	 * able to handle the CREATE_CHILD_SA response.
	 *
	 * @param child		CHILD_SA to check
	 * @return			TRUE if the SA is the redundant CHILD_SA
	 */
	bool (*is_redundant)(child_rekey_t *this, child_sa_t *child);

	/**
	 * Register a rekeying/delete task which collides with this one
	 *
	 * If two peers initiate rekeying at the same time, the collision must
	 * be handled gracefully. The task manager is aware of what exchanges
	 * are going on and notifies the active task by passing the passive.
	 *
	 * @param other		passive task (adopted)
	 */
	void (*collide)(child_rekey_t* this, task_t *other);
};

/**
 * Create a new TASK_CHILD_REKEY task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param protocol		protocol of CHILD_SA to rekey, PROTO_NONE as responder
 * @param spi			inbound SPI of CHILD_SA to rekey
 * @return				child_rekey task to handle by the task_manager
 */
child_rekey_t *child_rekey_create(ike_sa_t *ike_sa, protocol_id_t protocol,
								  uint32_t spi);

#endif /** CHILD_REKEY_H_ @}*/
