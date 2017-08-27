/*
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup ike_init ike_init
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_INIT_H_
#define IKE_INIT_H_

typedef struct ike_init_t ike_init_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type TASK_IKE_INIT, creates an IKE_SA without authentication.
 *
 * The authentication of is handle in the ike_auth task.
 */
struct ike_init_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;

	/**
	 * Get the lower of the two nonces, used for rekey collisions.
	 *
	 * @return			lower nonce
	 */
	chunk_t (*get_lower_nonce) (ike_init_t *this);
};

/**
 * Create a new TASK_IKE_INIT task.
 *
 * @param ike_sa		IKE_SA this task works for (new one when rekeying)
 * @param initiator		TRUE if task is the original initiator
 * @param old_sa		old IKE_SA when we are rekeying
 * @return				ike_init task to handle by the task_manager
 */
ike_init_t *ike_init_create(ike_sa_t *ike_sa, bool initiator, ike_sa_t *old_sa);

#endif /** IKE_INIT_H_ @}*/
