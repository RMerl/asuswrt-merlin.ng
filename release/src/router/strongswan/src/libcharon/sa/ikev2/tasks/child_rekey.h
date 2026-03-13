/*
 * Copyright (C) 2016-2022 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
typedef enum child_rekey_collision_t child_rekey_collision_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/child_sa.h>
#include <sa/task.h>

/**
 * Type of collision an active rekey task may have with an inbound DELETE.
 */
enum child_rekey_collision_t {
	/** Unrelated SA or unknown SPI (might be for the SA this task creates) */
	CHILD_REKEY_COLLISION_NONE = 0,
	/** Deleted SA is the one created by the peer in a collision */
	CHILD_REKEY_COLLISION_PEER,
	/** Deleted SA is the SA the active task is rekeying */
	CHILD_REKEY_COLLISION_OLD,
};

/**
 * Task of type TASK_CHILD_REKEY, rekey an established CHILD_SA.
 */
struct child_rekey_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;

	/**
	 * Handle a DELETE for the given CHILD_SA/SPI that might be related to
	 * this rekeiyng if the CREATE_CHILD_SA response is delayed.
	 *
	 * This checks if the given SA is the CHILD_SA created by the peer during a
	 * rekey collision or if it's for the old SA.
	 *
	 * If child is NULL, the SPI is collected as it might be for the SA this
	 * task is actively creating (the peer sends the inbound SA we don't know
	 * yet).
	 *
	 * @param child			CHILD_SA to check
	 * @param spi			SPI in case child is not known
	 * @return				type of collision
	 */
	child_rekey_collision_t (*handle_delete)(child_rekey_t *this,
											 child_sa_t *child, uint32_t spi);

	/**
	 * Register a rekey task which collides with this one.
	 *
	 * If two peers initiate rekeyings at the same time, the collision must
	 * be handled gracefully. The task manager is aware of what exchanges
	 * are going on and notifies the active task by passing the passive.
	 *
	 * @param other		passive task
	 * @return			whether the task was adopted and should be removed from
	 *					the task manager's control
	 */
	bool (*collide)(child_rekey_t* this, task_t *other);
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

/**
 * Conclude the rekeying for the given CHILD_SAs by installing the outbound
 * SA for the new CHILD_SA, uninstalling the one for the old and triggering
 * an appropriate log message and event.
 *
 * @param old			the old CHILD_SA
 * @param new			the new CHILD_SA
 * @return				TRUE if new outbound SA installed successfully
 */

bool child_rekey_conclude_rekeying(child_sa_t *old, child_sa_t *new);

#endif /** CHILD_REKEY_H_ @}*/
