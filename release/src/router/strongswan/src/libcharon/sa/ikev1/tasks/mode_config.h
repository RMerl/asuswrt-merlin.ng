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
 * @defgroup mode_config mode_config
 * @{ @ingroup tasks_v1
 */

#ifndef MODE_CONFIG_H_
#define MODE_CONFIG_H_

typedef struct mode_config_t mode_config_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type TASK_MODE_COFNIG, IKEv1 configuration attribute exchange.
 */
struct mode_config_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new mode_config task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE for initiator
 * @param pull			TRUE to pull, FALSE to push (applies if initiator only)
 * @return				mode_config task to handle by the task_manager
 */
mode_config_t *mode_config_create(ike_sa_t *ike_sa, bool initiator, bool pull);

#endif /** MODE_CONFIG_H_ @}*/
