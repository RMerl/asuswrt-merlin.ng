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
 * @defgroup certexpire_cron certexpire_cron
 * @{ @ingroup certexpire
 */

#ifndef CERTEXPIRE_CRON_H_
#define CERTEXPIRE_CRON_H_

typedef struct certexpire_cron_t certexpire_cron_t;

/**
 * Callback function invoked by cron.
 *
 * @param data		user specified callback data
 */
typedef void (*certexpire_cron_job_t)(void *data);

/**
 * Cron style job scheduling.
 */
struct certexpire_cron_t {

	/**
	 * Destroy a certexpire_cron_t.
	 *
	 * It currently is not possible to safely cancel a cron job. Make sure
	 * any scheduled jobs have been canceled before cleaning up.
	 */
	void (*destroy)(certexpire_cron_t *this);
};

/**
 * Create a certexpire_cron instance.
 *
 * The cron string takes numeric arguments only, but supports ranges (1-5)
 * and selections (1,3,5), or a combination, space separated:
 *  minute hour day month weekday
 *   minute, 0-59
 *   hour, 0-23
 *   day, 1-31
 *   month, 1-12
 *   weekday, 0-7 (0 == 7 == sunday)
 * man crontab(5) for details.
 *
 * @param cron		cron style scheduling string
 * @param job		callback function to invoke
 * @param data		user data to pass to job
 */
certexpire_cron_t *certexpire_cron_create(char *cron, certexpire_cron_job_t job,
										  void *data);

#endif /** CERTEXPIRE_CRON_H_ @}*/
