/*
 * Copyright (C) 2016-2020 Tobias Brunner
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
 * @defgroup ike_rekey ike_rekey
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_REKEY_H_
#define IKE_REKEY_H_

typedef struct ike_rekey_t ike_rekey_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type TASK_IKE_REKEY, rekey an established IKE_SA.
 */
struct ike_rekey_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;

	/**
	 * Check if there was a rekey collision.
	 *
	 * @return			TRUE if there was a rekey collision before
	 */
	bool (*did_collide)(ike_rekey_t *this);

	/**
	 * Register a rekeying/delete task which collides with this one.
	 *
	 * If two peers initiate rekeying at the same time, the collision must
	 * be handled gracefully. The task manager is aware of what exchanges
	 * are going on and notifies the active task by passing the passive.
	 *
	 * @param other		passive task
	 * @return			whether the task was adopted and should be removed from
	 *					the task manager's control
	 */
	bool (*collide)(ike_rekey_t* this, task_t *other);
};

/**
 * Create a new TASK_IKE_REKEY task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE for initiator, FALSE for responder
 * @return				TASK_IKE_REKEY task to handle by the task_manager
 */
ike_rekey_t *ike_rekey_create(ike_sa_t *ike_sa, bool initiator);

#endif /** IKE_REKEY_H_ @}*/
