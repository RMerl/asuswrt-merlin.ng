/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup task_manager_v1 task_manager_v1
 * @{ @ingroup ikev1
 */

#ifndef TASK_MANAGER_V1_H_
#define TASK_MANAGER_V1_H_

typedef struct task_manager_v1_t task_manager_v1_t;

#include <sa/task_manager.h>

/**
 * Task manager, IKEv1 variant.
 */
struct task_manager_v1_t {

	/**
	 * Implements task_manager_t.
	 */
	task_manager_t task_manager;
};

/**
 * Create an instance of the task manager.
 *
 * @param ike_sa		IKE_SA to manage.
 */
task_manager_v1_t *task_manager_v1_create(ike_sa_t *ike_sa);

/**
 * Check if the given CHILD_SA is redundant (i.e. another CHILD_SA with the same
 * name and TS is currently INSTALLED).
 *
 * Optionally, the two currently installed CHILD_SAs may be further compared.
 *
 * @param ike_sa		IKE_SA whose CHILD_SAs should be checked
 * @param child_sa		CHILD_SA to check for duplicates
 * @param cmp			Optional comparison function, first argument is the
 *						passed CHILD_SA, the second a found matching one, return
 *						TRUE if the former should be considered redundant
 * @return				TRUE if the CHILD_SA is redundant
 */
bool ikev1_child_sa_is_redundant(ike_sa_t *ike_sa, child_sa_t *child_sa,
								 bool (*cmp)(child_sa_t*,child_sa_t*));

#endif /** TASK_MANAGER_V1_H_ @}*/
