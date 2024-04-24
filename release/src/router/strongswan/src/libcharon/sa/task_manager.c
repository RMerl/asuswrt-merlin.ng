/*
 * Copyright (C) 2011-2023 Tobias Brunner
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

#include "task_manager.h"

#include <math.h>
#include <sa/ikev1/task_manager_v1.h>
#include <sa/ikev2/task_manager_v2.h>

/*
 * Described in header
 */
void retransmission_parse_default(retransmission_t *settings)
{
	settings->timeout = lib->settings->get_double(lib->settings,
					"%s.retransmit_timeout", RETRANSMIT_TIMEOUT, lib->ns);
	settings->base = lib->settings->get_double(lib->settings,
					"%s.retransmit_base", RETRANSMIT_BASE, lib->ns);
	settings->jitter = min(lib->settings->get_int(lib->settings,
					"%s.retransmit_jitter", 0, lib->ns), RETRANSMIT_JITTER_MAX);
	settings->limit = lib->settings->get_int(lib->settings,
					"%s.retransmit_limit", 0, lib->ns) * 1000;
	settings->tries = lib->settings->get_int(lib->settings,
					"%s.retransmit_tries", RETRANSMIT_TRIES, lib->ns);

	if (settings->base > 1)
	{	/* based on 1000 * timeout * base^try */
		settings->max_tries = log(UINT32_MAX/
								  (1000.0 * settings->timeout))/
							  log(settings->base);
	}
}

/*
 * Described in header
 */
uint32_t retransmission_timeout(retransmission_t *settings, u_int try,
								bool randomize)
{
	double timeout = UINT32_MAX, max_jitter;

	if (!settings->max_tries || try <= settings->max_tries)
	{
		timeout = settings->timeout * 1000.0 * pow(settings->base, try);
	}
	if (settings->limit)
	{
		timeout = min(timeout, settings->limit);
	}
	if (randomize && settings->jitter)
	{
		max_jitter = (timeout / 100.0) * settings->jitter;
		timeout -= max_jitter * (random() / (RAND_MAX + 1.0));
	}
	return (uint32_t)timeout;
}

/*
 * Described in header
 */
u_int retransmission_timeout_total(retransmission_t *settings)
{
	double total = 0;
	int i;

	for (i = 0; i <= settings->tries; i++)
	{
		total += retransmission_timeout(settings, i, FALSE) / 1000.0;
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
