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
 * @defgroup task_manager_v2 task_manager_v2
 * @{ @ingroup ikev2
 */

#ifndef TASK_MANAGER_V2_H_
#define TASK_MANAGER_V2_H_

typedef struct task_manager_v2_t task_manager_v2_t;

#include <sa/task_manager.h>

/**
 * Task manager, IKEv2 variant.
 */
struct task_manager_v2_t {

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
task_manager_v2_t *task_manager_v2_create(ike_sa_t *ike_sa);

#endif /** TASK_MANAGER_V2_H_ @}*/
