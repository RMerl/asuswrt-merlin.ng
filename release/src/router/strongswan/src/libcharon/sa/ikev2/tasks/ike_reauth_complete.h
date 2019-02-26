/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup ike_reauth_complete ike_reauth_complete
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_REAUTH_COMPLETE_H_
#define IKE_REAUTH_COMPLETE_H_

typedef struct ike_reauth_complete_t ike_reauth_complete_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type IKE_REAUTH_COMPLETE, removes reauthenticated SA after reauth.
 *
 * This task completes make-before-break reauthentication by deleting the
 * old, reauthenticated IKE_SA after the new one established.
 */
struct ike_reauth_complete_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new ike_reauth_complete task.
 *
 * This task is initiator only.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param id			old, reauthenticated IKE_SA
 * @return				ike_reauth_complete task to handle by the task_manager
 */
ike_reauth_complete_t *ike_reauth_complete_create(ike_sa_t *ike_sa,
												  ike_sa_id_t *id);

#endif /** IKE_REAUTH_COMPLETE_H_ @}*/
