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
 * @defgroup ike_delete ike_delete
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_DELETE_H_
#define IKE_DELETE_H_

typedef struct ike_delete_t ike_delete_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type ike_delete, delete an IKE_SA.
 */
struct ike_delete_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new ike_delete task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if we initiate the delete
 * @return				ike_delete task to handle by the task_manager
 */
ike_delete_t *ike_delete_create(ike_sa_t *ike_sa, bool initiator);

#endif /** IKE_DELETE_H_ @}*/
