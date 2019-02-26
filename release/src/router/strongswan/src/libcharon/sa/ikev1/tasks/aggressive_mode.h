/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup aggressive_mode aggressive_mode
 * @{ @ingroup tasks_v1
 */

#ifndef AGGRESSIVE_MODE_H_
#define AGGRESSIVE_MODE_H_

typedef struct aggressive_mode_t aggressive_mode_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * IKEv1 aggressive mode, establishes an IKE_SA without identity protection.
 */
struct aggressive_mode_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new AGGRESSIVE_MODE task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task initiated locally
 * @return				task to handle by the task_manager
 */
aggressive_mode_t *aggressive_mode_create(ike_sa_t *ike_sa, bool initiator);

#endif /** AGGRESSIVE_MODE_H_ @}*/
