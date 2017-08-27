/*
 *  Timer interface - local header file
 *  Copyright (c) 2001 by Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "local.h"

#ifndef DOC_HIDDEN
typedef struct {
	int (*close)(snd_timer_t *timer);
	int (*nonblock)(snd_timer_t *timer, int nonblock);
	int (*async)(snd_timer_t *timer, int sig, pid_t pid);
	int (*info)(snd_timer_t *timer, snd_timer_info_t *info);
	int (*params)(snd_timer_t *timer, snd_timer_params_t *params);
	int (*status)(snd_timer_t *timer, snd_timer_status_t *status);
	int (*rt_start)(snd_timer_t *timer);
	int (*rt_stop)(snd_timer_t *timer);
	int (*rt_continue)(snd_timer_t *timer);
	ssize_t (*read)(snd_timer_t *timer, void *buffer, size_t size);
} snd_timer_ops_t;

struct _snd_timer {
	unsigned int version;
	void *dl_handle;
	char *name;
	snd_timer_type_t type;
	int mode;
	int poll_fd;
	const snd_timer_ops_t *ops;
	void *private_data;
	struct list_head async_handlers;
};

typedef struct {
	int (*close)(snd_timer_query_t *timer);
	int (*next_device)(snd_timer_query_t *timer, snd_timer_id_t *tid);
	int (*info)(snd_timer_query_t *timer, snd_timer_ginfo_t *info);
	int (*params)(snd_timer_query_t *timer, snd_timer_gparams_t *info);
	int (*status)(snd_timer_query_t *timer, snd_timer_gstatus_t *info);
} snd_timer_query_ops_t;

struct _snd_timer_query {
	void *dl_handle;
	char *name;
	snd_timer_type_t type;
	int mode;
	int poll_fd;
	const snd_timer_query_ops_t *ops;
	void *private_data;
};
#endif /* DOC_HIDDEN */

int snd_timer_hw_open(snd_timer_t **handle, const char *name, int dev_class, int dev_sclass, int card, int device, int subdevice, int mode);

int snd_timer_query_hw_open(snd_timer_query_t **handle, const char *name, int mode);

int snd_timer_async(snd_timer_t *timer, int sig, pid_t pid);
