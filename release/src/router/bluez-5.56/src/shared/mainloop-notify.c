// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>

#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "mainloop.h"
#include "mainloop-notify.h"
#include "timeout.h"
#include "util.h"
#include "io.h"

#define WATCHDOG_TRIGGER_FREQ 2

static int notify_fd = -1;

static unsigned int watchdog;

struct signal_data {
	struct io *io;
	mainloop_signal_func func;
	void *user_data;
};

static struct signal_data *signal_data;

static bool watchdog_callback(void *user_data)
{
	mainloop_sd_notify("WATCHDOG=1");

	return true;
}

void mainloop_notify_init(void)
{
	const char *sock;
	struct sockaddr_un addr;
	const char *watchdog_usec;
	int msec;

	sock = getenv("NOTIFY_SOCKET");
	if (!sock)
		return;

	/* check for abstract socket or absolute path */
	if (sock[0] != '@' && sock[0] != '/')
		return;

	notify_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (notify_fd < 0)
		return;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock, sizeof(addr.sun_path) - 1);

	if (addr.sun_path[0] == '@')
		addr.sun_path[0] = '\0';

	if (connect(notify_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(notify_fd);
		notify_fd = -1;
		return;
	}

	watchdog_usec = getenv("WATCHDOG_USEC");
	if (!watchdog_usec)
		return;

	msec = atoi(watchdog_usec) / 1000;
	if (msec < 0)
		return;

	watchdog = timeout_add(msec / WATCHDOG_TRIGGER_FREQ,
				watchdog_callback, NULL, NULL);
}

void mainloop_notify_exit(void)
{
	if (notify_fd > 0) {
		close(notify_fd);
		notify_fd = -1;
	}

	timeout_remove(watchdog);
}

int mainloop_sd_notify(const char *state)
{
	int err;

	if (notify_fd <= 0)
		return -ENOTCONN;

	err = send(notify_fd, state, strlen(state), MSG_NOSIGNAL);
	if (err < 0)
		return -errno;

	return err;
}

static bool signal_read(struct io *io, void *user_data)
{
	struct signal_data *data = user_data;
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	fd = io_get_fd(io);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return false;

	if (data && data->func)
		data->func(si.ssi_signo, data->user_data);

	return true;
}

static struct io *setup_signalfd(void *user_data)
{
	struct io *io;
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGCHLD);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
		return NULL;

	fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
	if (fd < 0)
		return NULL;

	io = io_new(fd);

	io_set_close_on_destroy(io, true);
	io_set_read_handler(io, signal_read, user_data, free);

	return io;
}

int mainloop_run_with_signal(mainloop_signal_func func, void *user_data)
{
	struct signal_data *data;
	struct io *io;
	int ret;

	if (!func)
		return -EINVAL;

	data = new0(struct signal_data, 1);
	data->func = func;
	data->user_data = user_data;

	io = setup_signalfd(data);
	if (!io) {
		free(data);
		return -errno;
	}

	ret = mainloop_run();

	io_destroy(io);
	free(signal_data);

	return ret;
}
