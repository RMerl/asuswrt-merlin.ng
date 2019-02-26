/*
 * Copyright (C) 2011 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include <math.h>
#include <sa/ikev1/task_manager_v1.h>
#include <sa/ikev2/task_manager_v2.h>

/*
 * See header
 */
u_int task_manager_total_retransmit_timeout()
{
	double timeout, base, limit = 0, total = 0;
	int tries, i;

	tries = lib->settings->get_int(lib->settings, "%s.retransmit_tries",
								   RETRANSMIT_TRIES, lib->ns);
	base = lib->settings->get_double(lib->settings, "%s.retransmit_base",
									 RETRANSMIT_BASE, lib->ns);
	timeout = lib->settings->get_double(lib->settings, "%s.retransmit_timeout",
										RETRANSMIT_TIMEOUT, lib->ns);
	limit = lib->settings->get_double(lib->settings, "%s.retransmit_limit",
									  0, lib->ns);

	for (i = 0; i <= tries; i++)
	{
		double interval = timeout * pow(base, i);
		if (limit)
		{
			interval = min(interval, limit);
		}
		total += interval;
	}
	return (u_int)total;
}

/*
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

