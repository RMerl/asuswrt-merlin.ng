/*
 * Copyright (C) 2008 Martin Willi
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

#include "ha_ctl.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <threading/thread.h>
#include <processing/jobs/callback_job.h>

#define HA_FIFO IPSEC_PIDDIR "/charon.ha"

typedef struct private_ha_ctl_t private_ha_ctl_t;

/**
 * Private data of an ha_ctl_t object.
 */
struct private_ha_ctl_t {

	/**
	 * Public ha_ctl_t interface.
	 */
	ha_ctl_t public;

	/**
	 * Segments to control
	 */
	ha_segments_t *segments;

	/**
	 * Resynchronization message cache
	 */
	ha_cache_t *cache;
};

/**
 * FIFO dispatching function
 */
static job_requeue_t dispatch_fifo(private_ha_ctl_t *this)
{
	int fifo;
	bool oldstate;
	char buf[8];
	u_int segment;

	oldstate = thread_cancelability(TRUE);
	fifo = open(HA_FIFO, O_RDONLY);
	thread_cancelability(oldstate);
	if (fifo == -1)
	{
		DBG1(DBG_CFG, "opening HA fifo failed: %s", strerror(errno));
		sleep(1);
		return JOB_REQUEUE_FAIR;
	}

	memset(buf, 0, sizeof(buf));
	if (read(fifo, buf, sizeof(buf)-1) > 1)
	{
		segment = atoi(&buf[1]);
		if (segment)
		{
			switch (buf[0])
			{
				case '+':
					this->segments->activate(this->segments, segment, TRUE);
					break;
				case '-':
					this->segments->deactivate(this->segments, segment, TRUE);
					break;
				case '*':
					this->cache->resync(this->cache, segment);
					break;
				default:
					break;
			}
		}
	}
	close(fifo);

	return JOB_REQUEUE_DIRECT;
}

METHOD(ha_ctl_t, destroy, void,
	private_ha_ctl_t *this)
{
	free(this);
}

/**
 * See header
 */
ha_ctl_t *ha_ctl_create(ha_segments_t *segments, ha_cache_t *cache)
{
	private_ha_ctl_t *this;
	mode_t old;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.segments = segments,
		.cache = cache,
	);

	if (access(HA_FIFO, R_OK|W_OK) != 0)
	{
		old = umask(S_IRWXO);
		if (mkfifo(HA_FIFO, S_IRUSR | S_IWUSR) != 0)
		{
			DBG1(DBG_CFG, "creating HA FIFO %s failed: %s",
				 HA_FIFO, strerror(errno));
		}
		umask(old);
	}
	if (chown(HA_FIFO, lib->caps->get_uid(lib->caps),
			  lib->caps->get_gid(lib->caps)) != 0)
	{
		DBG1(DBG_CFG, "changing HA FIFO permissions failed: %s",
			 strerror(errno));
	}

	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create_with_prio((callback_job_cb_t)dispatch_fifo,
			this, NULL, (callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL));
	return &this->public;
}

