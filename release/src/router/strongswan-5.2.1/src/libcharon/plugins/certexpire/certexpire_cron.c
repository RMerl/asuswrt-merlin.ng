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

#include "certexpire_cron.h"

#include <time.h>

#include <utils/debug.h>
#include <processing/jobs/callback_job.h>

typedef struct private_certexpire_cron_t private_certexpire_cron_t;

/**
 * Private data of an certexpire_cron_t object.
 */
struct private_certexpire_cron_t {

	/**
	 * Public certexpire_cron_t interface.
	 */
	certexpire_cron_t public;

	/**
	 * time when to run export job
	 */
	struct {
		bool m[60];
		bool h[24];
		bool d[32];
		bool my[13];
		bool dw[8];
	} cron;

	/**
	 * Callback function to execute
	 */
	certexpire_cron_job_t job;

	/**
	 * Data to pass to callback
	 */
	void *data;
};

/**
 * Check if we should execute the export job
 */
static job_requeue_t check_cron(private_certexpire_cron_t *this)
{
	struct tm tm;
	time_t t;

	t = time(NULL);
	localtime_r(&t, &tm);

	/* recheck every minute at second 0 */
	lib->scheduler->schedule_job(lib->scheduler,
			(job_t*)callback_job_create_with_prio((callback_job_cb_t)check_cron,
			this, NULL, NULL, JOB_PRIO_CRITICAL), 60 - tm.tm_sec);

	/* skip this minute if we had a large negative time shift */
	if (tm.tm_sec <= 30)
	{
		if (this->cron.m[tm.tm_min] &&
			this->cron.h[tm.tm_hour] &&
			this->cron.d[tm.tm_mday] &&
			this->cron.my[tm.tm_mon + 1] &&
			(this->cron.dw[tm.tm_wday] ||
			 (this->cron.dw[7] && tm.tm_wday == 0)))
		{
			this->job(this->data);
		}
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Parse a cron range component into boolean fields
 */
static void parse_ranges(bool *fields, char *label, int mi, int ma, char *range)
{
	enumerator_t *enumerator;
	int from, to, i;

	if (streq(range, "*"))
	{
		for (i = mi; i <= ma; i++)
		{
			fields[i] = TRUE;
		}
	}
	else
	{
		enumerator = enumerator_create_token(range, ",", "");
		while (enumerator->enumerate(enumerator, &range))
		{
			switch (sscanf(range, "%d-%d", &from, &to))
			{
				case 1: /* single value */
					if (from >= mi && from <= ma)
					{
						fields[from] = TRUE;
					}
					else
					{
						DBG1(DBG_CFG, "ignoring cron %s %d, out of range",
							 label, from);
					}
					break;
				case 2: /* range */
					if (from < mi)
					{
						DBG1(DBG_CFG, "cron %s out of range, shortening start "
							 "from %d to %d", label, from, mi);
						from = mi;
					}
					if (to > ma)
					{
						DBG1(DBG_CFG, "cron %s out of range, shortening end "
							 "from %d to %d", label, to, ma);
						to = ma;
					}
					for (i = from; i <= to; i++)
					{
						fields[i] = TRUE;
					}
					break;
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);
	}
	DBG3(DBG_CFG, "cron job with enabled %ss:", label);
	for (i = mi; i <= ma; i++)
	{
		if (fields[i])
		{
			DBG3(DBG_CFG, "  %d", i);
		}
	}
}

/**
 * Start cron processing, if configured
 */
static void start_cron(private_certexpire_cron_t *this, char *cron)
{
	enumerator_t *enumerator;
	int i = 0;

	enumerator = enumerator_create_token(cron, " ", " ");
	for (i = 0; i < 5; i++)
	{
		if (!enumerator->enumerate(enumerator, &cron))
		{
			DBG1(DBG_CFG, "cron misses a field, using '*'");
			cron = "*";
		}
		switch (i)
		{
			case 0:
				parse_ranges(this->cron.m, "minute", 0, 59, cron);
				break;
			case 1:
				parse_ranges(this->cron.h, "hour", 0, 23, cron);
				break;
			case 2:
				parse_ranges(this->cron.d, "day", 1, 31, cron);
				break;
			case 3:
				parse_ranges(this->cron.my, "month", 1, 12, cron);
				break;
			case 4:
				parse_ranges(this->cron.dw, "weekday", 0, 7, cron);
				break;
			default:
				break;
		}
	}
	if (enumerator->enumerate(enumerator, &cron))
	{
		DBG1(DBG_CFG, "ignoring extra fields in cron");
	}
	enumerator->destroy(enumerator);

	check_cron(this);
}

METHOD(certexpire_cron_t, destroy, void,
	private_certexpire_cron_t *this)
{
	free(this);
}

/**
 * See header
 */
certexpire_cron_t *certexpire_cron_create(char *cron, certexpire_cron_job_t job,
										  void *data)
{
	private_certexpire_cron_t *this;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.job = job,
		.data = data,
	);

	start_cron(this, cron);

	return &this->public;
}
