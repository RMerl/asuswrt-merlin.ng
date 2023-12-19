/*
 * Copyright (c) 2017 Gleb Smirnoff <glebius@FreeBSD.org>
 * Copyright (c) 2002-2017 Igor Sysoev
 * Copyright (c) 2011-2017 Nginx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "event.h"
#include "log.h"

static event_module_init_t select_init;
static event_module_fini_t select_fini;
static event_module_add_t select_add;
static event_module_del_t select_del;
static event_module_process_t select_process;

static fd_set master_read_fd_set;
static fd_set master_write_fd_set;
static fd_set work_read_fd_set;
static fd_set work_write_fd_set;

static struct event **events;
static int nevents;
static int max_fd;

struct event_module event_module = {
	.add =		select_add,
	.del =		select_del,
	.process =	select_process,
	.init = 	select_init,
	.fini =		select_fini,
};

static int
select_init(void)
{

	events = calloc(FD_SETSIZE, sizeof(struct event *));
	if (events == NULL)
		return (ENOMEM);

	FD_ZERO(&master_read_fd_set);
	FD_ZERO(&master_write_fd_set);
	max_fd = 0;
	nevents = 0;

	return (0);
}


static void
select_fini(void)
{

	free(events);
	events = NULL;
}

static int
select_add(struct event *ev)
{

	assert(ev->fd < FD_SETSIZE);

	switch (ev->rdwr) {
	case EVENT_READ:
		FD_SET(ev->fd, &master_read_fd_set);
		break;
	case EVENT_WRITE:
		FD_SET(ev->fd, &master_write_fd_set);
		break;
	}

	if (max_fd != -1 && max_fd < ev->fd)
		max_fd = ev->fd;

	events[nevents] = ev;
	ev->index = nevents++;

	assert(nevents < FD_SETSIZE);

	return (0);
}

static int
select_del(struct event *ev, int flags)
{

	assert(ev->fd < FD_SETSIZE);

	switch (ev->rdwr) {
	case EVENT_READ:
		FD_CLR(ev->fd, &master_read_fd_set);
		break;
	case EVENT_WRITE:
		FD_CLR(ev->fd, &master_write_fd_set);
		break;
	}

	if (max_fd == ev->fd)
		max_fd = -1;

	if (events != NULL && (ev->index < --nevents)) {
		struct event *ev0;

		ev0 = events[nevents];
		events[ev->index] = ev0;
		ev0->index = ev->index;
	}
	ev->index = -1;

	return (0);
}

static int
select_process(struct timeval *tv)
{
	struct event *ev;
	int ready, i;

	/* Need to rescan for max_fd. */
	if (max_fd == -1)
		for (i = 0; i < nevents; i++) {
			if (max_fd < events[i]->fd)
				max_fd = events[i]->fd;
		}

	work_read_fd_set = master_read_fd_set;
	work_write_fd_set = master_write_fd_set;

	ready = select(max_fd + 1, &work_read_fd_set, &work_write_fd_set, NULL, tv);

	if (ready == -1) {
		if (errno == EINTR)
			return (errno);
		DPRINTF(E_FATAL, L_GENERAL, "select(): %s. EXITING\n", strerror(errno));
	}

	if (ready == 0)
		return (0);

	for (i = 0; i < nevents; i++) {
		ev = events[i];

		switch (ev->rdwr) {
		case EVENT_READ:
			if (FD_ISSET(ev->fd, &work_read_fd_set))
				ev->up.process(ev);
			break;
		case EVENT_WRITE:
			if (FD_ISSET(ev->fd, &work_write_fd_set))
				ev->up.process(ev);
			break;
		}
	}

	return (0);
}
