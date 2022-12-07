/*
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
 * @defgroup ike_reauth ike_reauth
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_REAUTH_H_
#define IKE_REAUTH_H_

typedef struct ike_reauth_t ike_reauth_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type ike_reauth, reestablishes an IKE_SA.
 *
 * This task implements break-before-make reauthentication.
 */
struct ike_reauth_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new ike_reauth task.
 *
 * This task is initiator only.
 *
 * @param ike_sa		IKE_SA this task works for
 * @return				ike_reauth task to handle by the task_manager
 */
ike_reauth_t *ike_reauth_create(ike_sa_t *ike_sa);

#endif /** IKE_REAUTH_H_ @}*/
