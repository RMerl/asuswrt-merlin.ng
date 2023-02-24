/*
 * Copyright (C) 2022 Tobias Brunner
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
 * @defgroup ike_establish ike_establish
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_ESTABLISH_H_
#define IKE_ESTABLISH_H_

typedef struct ike_establish_t ike_establish_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type TASK_IKE_ESTABLISH that sets the state of the IKE_SA to
 * IKE_ESTABLISHED and triggers the ike_updown() event.
 */
struct ike_establish_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new TASK_IKE_ESTABLISH task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task is initiated by us
 * @return				created task
 */
ike_establish_t *ike_establish_create(ike_sa_t *ike_sa, bool initiator);

#endif /** IKE_ESTABLISH_H_ @}*/
