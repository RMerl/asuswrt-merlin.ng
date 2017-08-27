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
 * @defgroup main_mode main_mode
 * @{ @ingroup tasks_v1
 */

#ifndef MAIN_MODE_H_
#define MAIN_MODE_H_

typedef struct main_mode_t main_mode_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * IKEv1 main mode, establishes a mainmode including authentication.
 */
struct main_mode_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new main_mode task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task initiated locally
 * @return				task to handle by the task_manager
 */
main_mode_t *main_mode_create(ike_sa_t *ike_sa, bool initiator);

#endif /** MAIN_MODE_H_ @}*/
