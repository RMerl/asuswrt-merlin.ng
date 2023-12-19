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
#include <sys/event.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "event.h"
#include "log.h"

static int kqueue_set(struct event *, short, u_short, u_int);

static event_module_init_t kqueue_init;
static event_module_fini_t kqueue_fini;
static event_module_add_t kqueue_add;
static event_module_del_t kqueue_del;
static event_module_process_t kqueue_process;

static int kq;
static struct kevent *change_list;
static struct kevent *event_list;
static u_int nchanges;

#define	MAXCHANGES	128
#define	MAXEVENTS	128

struct event_module event_module = {
	.add =		kqueue_add,
	.del =		kqueue_del,
	.process =	kqueue_process,
	.init =		kqueue_init,
	.fini =		kqueue_fini,
};

static int
kqueue_init(void)
{

	kq = kqueue();
	if (kq == -1)
		return (errno);

	change_list = calloc(MAXCHANGES, sizeof(struct kevent));
	event_list = calloc(MAXEVENTS, sizeof(struct kevent));
	if (change_list == NULL || event_list == NULL)
		return (ENOMEM);

	nchanges = 0;

	return (0);
}

static void
kqueue_fini()
{

	(void )close(kq);
	kq = -1;

	free(change_list);
	free(event_list);
	change_list = NULL;
	event_list = NULL;
	nchanges = 0;
}

static int
kqueue_add(struct event *ev)
{
	u_int fflags;
	u_short flags;

	if (ev->rdwr == EVFILT_VNODE) {
		flags = EV_ADD | EV_ENABLE | EV_CLEAR;
		fflags = NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND;
	} else {
		flags = EV_ADD | EV_ENABLE;
		fflags = 0;
	}

	DPRINTF(E_DEBUG, L_GENERAL, "kqueue_add %d\n", ev->fd);
	return (kqueue_set(ev, ev->rdwr, flags, fflags));
}

static int
kqueue_del(struct event *ev, int flags)
{

        /*
	 * If the event is still not passed to a kernel,
	 * we will not pass it.
	 */
	assert(ev->fd >= 0);
	if (ev->index < nchanges &&
	    change_list[ev->index].udata == ev) {
		if (ev->index < --nchanges) {
			struct event *ev0;

			ev0 = (struct event *)change_list[nchanges].udata;
			change_list[ev->index] = change_list[nchanges];
			ev0->index = ev->index;
		}
		return (0);
	}

	/*
	 * when the file descriptor is closed the kqueue automatically deletes
	 * its filters so we do not need to delete explicitly the event
	 * before the closing the file descriptor.
	 */
	if (flags & EV_FLAG_CLOSING)
		return (0);

	DPRINTF(E_DEBUG, L_GENERAL, "kqueue_del %d\n", ev->fd);
	return (kqueue_set(ev, ev->rdwr, EV_DELETE, 0));
}

static int
kqueue_set(struct event *ev, short filter, u_short flags, u_int fflags)
{
	struct kevent *kev;
	struct timespec ts;

	if (nchanges >= MAXCHANGES) {
		DPRINTF(E_INFO, L_GENERAL, "kqueue change list is filled up\n");

		ts.tv_sec = 0;
		ts.tv_nsec = 0;

		if (kevent(kq, change_list, (int) nchanges, NULL, 0, &ts) == -1) {
			DPRINTF(E_ERROR, L_GENERAL,"kevent() failed: %s\n", strerror(errno));
			return (errno);
		}
		nchanges = 0;
	}

	kev = &change_list[nchanges];
	kev->ident = ev->fd;
	kev->filter = filter;
	kev->flags = flags;
	kev->udata = ev;
	kev->fflags = fflags;
	kev->data = 0;

	ev->index = nchanges++;

	return (0);
}

static int
kqueue_process(struct timeval *tv)
{
	struct event *ev;
	int events, n, i;
	struct timespec ts;

	n = (int) nchanges;
	nchanges = 0;

	TIMEVAL_TO_TIMESPEC(tv, &ts);

	DPRINTF(E_DEBUG, L_GENERAL, "kevent timer: %lu.%06lu, changes: %d\n",
	    ts.tv_sec, ts.tv_nsec, n);

	events = kevent(kq, change_list, n, event_list, MAXEVENTS, &ts);

	if (events == -1) {
		if (errno == EINTR)
			return (errno);
		DPRINTF(E_FATAL, L_GENERAL, "kevent(): %s. EXITING\n", strerror(errno));
	}

	DPRINTF(E_DEBUG, L_GENERAL, "kevent events: %d\n", events);

	for (i = 0; i < events; i++) {
		if (event_list[i].flags & EV_ERROR) {
			DPRINTF(E_ERROR, L_GENERAL,
			    "kevent() error %d on %d filter:%d flags:0x%x\n",
			    (int)event_list[i].data, (int)event_list[i].ident,
			    event_list[i].filter, event_list[i].flags);
			continue;
		}

		ev = (struct event *)event_list[i].udata;

		switch (event_list[i].filter) {
		case EVFILT_READ:
		case EVFILT_WRITE:
			ev->process(ev);
			break;
		case EVFILT_VNODE:
			ev->process_vnode(ev, event_list[i].fflags);
			break;
		default:
			DPRINTF(E_ERROR, L_GENERAL,
			    "unexpected kevent() filter %d",
			    event_list[i].filter);
			continue;
		}
	}

	return (0);
}
