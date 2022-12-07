/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "signal.h"
#include "queue.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "private.h"
#include "timeout.h"

/**
 * SECTION:main
 * @short_description: Main loop handling
 *
 * Main loop handling
 */

#define MAX_EPOLL_EVENTS 10

#define IDLE_FLAG_DISPATCHING	1
#define IDLE_FLAG_DESTROYED	2

#define WATCH_FLAG_DISPATCHING	1
#define WATCH_FLAG_DESTROYED	2

#define WATCHDOG_TRIGGER_FREQ	2

static int epoll_fd;
static bool epoll_running;
static bool epoll_terminate;
static int idle_id;

static int notify_fd;

static struct l_timeout *watchdog;

static struct l_queue *idle_list;

struct watch_data {
	int fd;
	uint32_t events;
	uint32_t flags;
	watch_event_cb_t callback;
	watch_destroy_cb_t destroy;
	void *user_data;
};

#define DEFAULT_WATCH_ENTRIES 128

static unsigned int watch_entries;
static struct watch_data **watch_list;

struct idle_data {
	idle_event_cb_t callback;
	idle_destroy_cb_t destroy;
	void *user_data;
	uint32_t flags;
	int id;
};

static inline bool __attribute__ ((always_inline)) create_epoll(void)
{
	unsigned int i;

	epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if (epoll_fd < 0) {
		epoll_fd = 0;
		return false;
	}

	watch_list = malloc(DEFAULT_WATCH_ENTRIES * sizeof(void *));
	if (!watch_list)
		goto close_epoll;

	idle_list = l_queue_new();

	idle_id = 0;

	watch_entries = DEFAULT_WATCH_ENTRIES;

	for (i = 0; i < watch_entries; i++)
		watch_list[i] = NULL;

	return true;

close_epoll:
	close(epoll_fd);
	epoll_fd = 0;

	return false;
}

int watch_add(int fd, uint32_t events, watch_event_cb_t callback,
				void *user_data, watch_destroy_cb_t destroy)
{
	struct watch_data *data;
	struct epoll_event ev;
	int err;

	if (unlikely(fd < 0 || !callback))
		return -EINVAL;

	if (!epoll_fd)
		return -EIO;

	if ((unsigned int) fd > watch_entries - 1)
		return -ERANGE;

	data = l_new(struct watch_data, 1);

	data->fd = fd;
	data->events = events;
	data->flags = 0;
	data->callback = callback;
	data->destroy = destroy;
	data->user_data = user_data;

	memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = data;

	err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data->fd, &ev);
	if (err < 0) {
		l_free(data);
		return -errno;
	}

	watch_list[fd] = data;

	return 0;
}

int watch_modify(int fd, uint32_t events, bool force)
{
	struct watch_data *data;
	struct epoll_event ev;
	int err;

	if (unlikely(fd < 0))
		return -EINVAL;

	if ((unsigned int) fd > watch_entries - 1)
		return -ERANGE;

	data = watch_list[fd];
	if (!data)
		return -ENXIO;

	if (data->events == events && !force)
		return 0;

	memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = data;

	err = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, data->fd, &ev);
	if (err < 0)
		return -errno;

	data->events = events;

	return 0;
}

int watch_clear(int fd)
{
	struct watch_data *data;

	if (unlikely(fd < 0))
		return -EINVAL;

	if ((unsigned int) fd > watch_entries - 1)
		return -ERANGE;

	data = watch_list[fd];
	if (!data)
		return -ENXIO;

	watch_list[fd] = NULL;

	if (data->destroy)
		data->destroy(data->user_data);

	if (data->flags & WATCH_FLAG_DISPATCHING)
		data->flags |= WATCH_FLAG_DESTROYED;
	else
		l_free(data);

	return 0;
}

int watch_remove(int fd, bool epoll_del)
{
	int err = watch_clear(fd);

	if (err < 0)
		return err;

	if (!epoll_del)
		goto done;

	err = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	if (err < 0)
		return -errno;

done:
	return err;
}

static bool idle_remove_by_id(void *data, void *user_data)
{
	struct idle_data *idle = data;
	int id = L_PTR_TO_INT(user_data);

	if (idle->id != id)
		return false;

	if (idle->destroy)
		idle->destroy(idle->user_data);

	if (idle->flags & IDLE_FLAG_DISPATCHING) {
		idle->flags |= IDLE_FLAG_DESTROYED;
		return false;
	}

	l_free(idle);

	return true;
}

static bool idle_prune(void *data, void *user_data)
{
	struct idle_data *idle = data;

	if ((idle->flags & IDLE_FLAG_DESTROYED) == 0)
		return false;

	l_free(idle);

	return true;
}

int idle_add(idle_event_cb_t callback, void *user_data, uint32_t flags,
		idle_destroy_cb_t destroy)
{
	struct idle_data *data;

	if (unlikely(!callback))
		return -EINVAL;

	if (!epoll_fd)
		return -EIO;

	data = l_new(struct idle_data, 1);

	data->callback = callback;
	data->destroy = destroy;
	data->user_data = user_data;
	data->flags = flags;

	if (!l_queue_push_tail(idle_list, data)) {
		l_free(data);
		return -ENOMEM;
	}

	data->id = idle_id++;

	if (idle_id == INT_MAX)
		idle_id = 0;

	return data->id;
}

void idle_remove(int id)
{
	l_queue_foreach_remove(idle_list, idle_remove_by_id,
					L_INT_TO_PTR(id));
}

static void idle_destroy(void *data)
{
	struct idle_data *idle = data;

	if (!(idle->flags & IDLE_FLAG_NO_WARN_DANGLING))
		l_error("Dangling idle descriptor %p, %d found",
							data, idle->id);

	if (idle->destroy)
		idle->destroy(idle->user_data);

	l_free(idle);
}

static void idle_dispatch(void *data, void *user_data)
{
	struct idle_data *idle = data;

	if (!idle->callback)
		return;

	idle->flags |= IDLE_FLAG_DISPATCHING;
	idle->callback(idle->user_data);
	idle->flags &= ~IDLE_FLAG_DISPATCHING;
}

static int sd_notify(const char *state)
{
	int err;

	if (notify_fd <= 0)
		return -ENOTCONN;

	err = send(notify_fd, state, strlen(state), MSG_NOSIGNAL);
	if (err < 0)
		return -errno;

	return 0;
}

static void watchdog_callback(struct l_timeout *timeout, void *user_data)
{
	int msec = L_PTR_TO_INT(user_data);

	sd_notify("WATCHDOG=1");

	l_timeout_modify_ms(timeout, msec);
}

static void create_sd_notify_socket(void)
{
	const char *sock;
	struct sockaddr_un addr;
	const char *watchdog_usec;
	int msec;

	/* check if NOTIFY_SOCKET has been set */
	sock = getenv("NOTIFY_SOCKET");
	if (!sock)
		return;

	/* check for abstract socket or absolute path */
	if (sock[0] != '@' && sock[0] != '/')
		return;

	notify_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (notify_fd < 0) {
		notify_fd = 0;
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock, sizeof(addr.sun_path) - 1);

	if (addr.sun_path[0] == '@')
		addr.sun_path[0] = '\0';

	if (bind(notify_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(notify_fd);
		notify_fd = 0;
		return;
	}

	watchdog_usec = getenv("WATCHDOG_USEC");
	if (!watchdog_usec)
		return;

	msec = atoi(watchdog_usec) / 1000;
	if (msec < WATCHDOG_TRIGGER_FREQ)
		return;

	msec /= WATCHDOG_TRIGGER_FREQ;

	watchdog = l_timeout_create_ms(msec, watchdog_callback,
					L_INT_TO_PTR(msec), NULL);
}

/**
 * l_main_init:
 *
 * Initialize the main loop. This must be called before l_main_run()
 * and any other function that directly or indirectly sets up an idle
 * or watch. A safe rule-of-thumb is to call it before any function
 * prefixed with "l_".
 *
 * Returns: true if initialization was successful, false otherwise.
 **/
LIB_EXPORT bool l_main_init(void)
{
	if (unlikely(epoll_running))
		return false;

	if (!create_epoll())
		return false;

	create_sd_notify_socket();

	epoll_terminate = false;

	return true;
}

/**
 * l_main_prepare:
 *
 * Prepare the iteration of the main loop
 *
 * Returns: The timeout to use.  This will be 0 if idle-event processing is
 * currently pending, or -1 otherwise.  This value can be used to pass to
 * l_main_iterate.
 */
LIB_EXPORT int l_main_prepare(void)
{
	return l_queue_isempty(idle_list) ? -1 : 0;
}

/**
 * l_main_iterate:
 *
 * Run one iteration of the main event loop
 */
LIB_EXPORT void l_main_iterate(int timeout)
{
	struct epoll_event events[MAX_EPOLL_EVENTS];
	struct watch_data *data;
	int n, nfds;

	nfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, timeout);

	for (n = 0; n < nfds; n++) {
		data = events[n].data.ptr;

		data->flags |= WATCH_FLAG_DISPATCHING;
	}

	for (n = 0; n < nfds; n++) {
		data = events[n].data.ptr;

		if (data->flags & WATCH_FLAG_DESTROYED)
			continue;

		data->callback(data->fd, events[n].events,
							data->user_data);
	}

	for (n = 0; n < nfds; n++) {
		data = events[n].data.ptr;

		if (data->flags & WATCH_FLAG_DESTROYED)
			l_free(data);
		else
			data->flags = 0;
	}

	l_queue_foreach(idle_list, idle_dispatch, NULL);
	l_queue_foreach_remove(idle_list, idle_prune, NULL);
}

/**
 * l_main_run:
 *
 * Run the main loop
 *
 * The loop may be restarted by invoking this function after a
 * previous invocation returns, provided that l_main_exit() has not
 * been called first.
 *
 * Returns: #EXIT_SUCCESS after successful execution or #EXIT_FAILURE in
 *          case of failure
 **/
LIB_EXPORT int l_main_run(void)
{
	int timeout;

	/* Has l_main_init() been called? */
	if (unlikely(!epoll_fd))
		return EXIT_FAILURE;

	if (unlikely(epoll_running))
		return EXIT_FAILURE;

	epoll_running = true;

	for (;;) {
		if (epoll_terminate)
			break;

		timeout = l_main_prepare();
		l_main_iterate(timeout);
	}

	epoll_running = false;

	if (notify_fd) {
		close(notify_fd);
		notify_fd = 0;
		l_timeout_remove(watchdog);
		watchdog = NULL;
	}

	return EXIT_SUCCESS;
}

/**
 * l_main_exit:
 *
 * Clean up after main loop completes.
 *
 **/
LIB_EXPORT bool l_main_exit(void)
{
	unsigned int i;

	if (epoll_running) {
		l_error("Cleanup attempted on running main loop");
		return false;
	}

	for (i = 0; i < watch_entries; i++) {
		struct watch_data *data = watch_list[i];

		if (!data)
			continue;

		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, data->fd, NULL);

		if (data->destroy)
			data->destroy(data->user_data);
		else
			l_error("Dangling file descriptor %d found", data->fd);

		l_free(data);
	}

	watch_entries = 0;

	free(watch_list);
	watch_list = NULL;

	l_queue_destroy(idle_list, idle_destroy);
	idle_list = NULL;

	close(epoll_fd);
	epoll_fd = 0;

	return true;
}

/**
 * l_main_quit:
 *
 * Teminate the running main loop
 *
 * Returns: #true when terminating the main loop or #false in case of failure
 **/
LIB_EXPORT bool l_main_quit(void)
{
	if (unlikely(!epoll_running))
		return false;

	epoll_terminate = true;

	return true;
}

struct signal_data {
	l_main_signal_cb_t callback;
	void *user_data;
};

static void sigint_handler(void *user_data)
{
	struct signal_data *data = user_data;

	if (data->callback)
		data->callback(SIGINT, data->user_data);
}

static void sigterm_handler(void *user_data)
{
	struct signal_data *data = user_data;

	if (data->callback)
		data->callback(SIGTERM, data->user_data);
}

/**
 * l_main_run_with_signal:
 *
 * Run the main loop with signal handling for SIGINT and SIGTERM
 *
 * Returns: #EXIT_SUCCESS after successful execution or #EXIT_FAILURE in
 *          case of failure
 **/
LIB_EXPORT int l_main_run_with_signal(l_main_signal_cb_t callback,
							void *user_data)
{
	struct signal_data *data;
	struct l_signal *sigint;
	struct l_signal *sigterm;
	int result;

	data = l_new(struct signal_data, 1);

	data->callback = callback;
	data->user_data = user_data;

	sigint = l_signal_create(SIGINT, sigint_handler, data, NULL);
	sigterm = l_signal_create(SIGTERM, sigterm_handler, data, NULL);

	result = l_main_run();

	l_signal_remove(sigint);
	l_signal_remove(sigterm);

	l_free(data);

	return result;
}

/**
 * l_main_get_epoll_fd:
 *
 * Can be used to obtain the epoll file descriptor in order to integrate
 * the ell main event loop with other event loops.
 *
 * Returns: epoll file descriptor
 **/
LIB_EXPORT int l_main_get_epoll_fd(void)
{
	return epoll_fd;
}
