/*
 * Copyright (C) 2011 Tobias Brunner
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

#include "task_manager.h"

#include <sa/ikev1/task_manager_v1.h>
#include <sa/ikev2/task_manager_v2.h>

/**
 * See header
 */
task_manager_t *task_manager_create(ike_sa_t *ike_sa)
{
	switch (ike_sa->get_version(ike_sa))
	{
		case IKEV1:
#ifdef USE_IKEV1
			return &task_manager_v1_create(ike_sa)->task_manager;
#endif
			break;
		case IKEV2:
#ifdef USE_IKEV2
			return &task_manager_v2_create(ike_sa)->task_manager;
#endif
			break;
		default:
			break;
	}
	return NULL;
}

