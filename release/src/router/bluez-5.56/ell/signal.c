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
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>

#include "util.h"
#include "io.h"
#include "queue.h"
#include "signal.h"
#include "private.h"

/**
 * SECTION:signal
 * @short_description: Unix signal support
 *
 * Unix signal support
 */

/**
 * l_signal:
 *
 * Opague object representing the signal.
 */
struct l_signal {
	struct signal_desc *desc;
	l_signal_notify_cb_t callback;
	void *user_data;
	l_signal_destroy_cb_t destroy;
};

struct signal_desc {
	uint32_t signo;
	struct l_queue *callbacks;
};

static struct l_io *signalfd_io = NULL;
static struct l_queue *signal_list = NULL;
static sigset_t signal_mask;

static void handle_callback(struct signal_desc *desc)
{
	const struct l_queue_entry *entry;

	for (entry = l_queue_get_entries(desc->callbacks); entry;
							entry = entry->next) {
		struct l_signal *signal = entry->data;

		if (signal->callback)
			signal->callback(signal->user_data);
	}
}

static bool desc_match_signo(const void *a, const void *b)
{
	const struct signal_desc *desc = a;
	uint32_t signo = L_PTR_TO_UINT(b);

	return (desc->signo == signo);
}

static bool signalfd_read_cb(struct l_io *io, void *user_data)
{
	int fd = l_io_get_fd(io);
	struct signal_desc *desc;
	struct signalfd_siginfo si;
	ssize_t result;

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return true;

	desc = l_queue_find(signal_list, desc_match_signo,
						L_UINT_TO_PTR(si.ssi_signo));
	if (desc)
		handle_callback(desc);

	return true;
}

static bool signalfd_add(int signo)
{
	int fd;

	if (!signalfd_io) {
		fd = -1;
		sigemptyset(&signal_mask);
	} else
		fd = l_io_get_fd(signalfd_io);

	sigaddset(&signal_mask, signo);

	fd = signalfd(fd, &signal_mask, SFD_CLOEXEC);
	if (fd < 0)
		return false;

	if (signalfd_io)
		return true;

	signalfd_io = l_io_new(fd);
	if (!signalfd_io) {
		close(fd);
		return false;
	}

	l_io_set_close_on_destroy(signalfd_io, true);

	if (!l_io_set_read_handler(signalfd_io, signalfd_read_cb, NULL, NULL)) {
		l_io_destroy(signalfd_io);
		return false;
	}

	signal_list = l_queue_new();

	return true;
}

static void signalfd_remove(int signo)
{
	if (!signalfd_io)
		return;

	sigdelset(&signal_mask, signo);

	if (!sigisemptyset(&signal_mask)) {
		signalfd(l_io_get_fd(signalfd_io), &signal_mask, SFD_CLOEXEC);
		return;
	}

	l_io_destroy(signalfd_io);
	signalfd_io = NULL;

	l_queue_destroy(signal_list, NULL);
	signal_list = NULL;
}

/**
 * l_signal_create:
 * @callback: signal callback function
 * @user_data: user data provided to signal callback function
 * @destroy: destroy function for user data
 *
 * Create new signal callback handling for a given set of signals.
 *
 * Returns: a newly allocated #l_signal object
 **/
LIB_EXPORT struct l_signal *l_signal_create(uint32_t signo,
				l_signal_notify_cb_t callback,
				void *user_data, l_signal_destroy_cb_t destroy)
{
	struct l_signal *signal;
	struct signal_desc *desc;
	sigset_t mask, oldmask;

	if (signo <= 1 || signo >= _NSIG)
		return NULL;

	signal = l_new(struct l_signal, 1);
	signal->callback = callback;
	signal->destroy = destroy;
	signal->user_data = user_data;

	desc = l_queue_find(signal_list, desc_match_signo,
						L_UINT_TO_PTR(signo));
	if (desc)
		goto done;

	sigemptyset(&mask);
	sigaddset(&mask, signo);

	if (sigprocmask(SIG_BLOCK, &mask, &oldmask) < 0) {
		l_free(signal);
		return NULL;
	}

	if (!signalfd_add(signo)) {
		sigprocmask(SIG_SETMASK, &oldmask, NULL);
		l_free(signal);
		return NULL;
	}

	desc = l_new(struct signal_desc, 1);
	desc->signo = signo;
	desc->callbacks = l_queue_new();

	l_queue_push_tail(signal_list, desc);

done:
	l_queue_push_tail(desc->callbacks, signal);
	signal->desc = desc;
	return signal;
}

/**
 * l_signal_remove:
 * @signal: signal object
 *
 * Remove signal handling.
 **/
LIB_EXPORT void l_signal_remove(struct l_signal *signal)
{
	struct signal_desc *desc;
	sigset_t mask;

	if (!signal)
		return;

	desc = signal->desc;
	l_queue_remove(desc->callbacks, signal);

	/*
	 * As long as the signal descriptor has callbacks registered, it is
	 * still needed to be active.
	 */
	if (!l_queue_isempty(desc->callbacks))
		goto done;

	if (!l_queue_remove(signal_list, desc))
		goto done;

	sigemptyset(&mask);
	sigaddset(&mask, desc->signo);

	/*
	 * When the number of signals goes to zero, then this will close
	 * the signalfd file descriptor, otherwise it will only adjust the
	 * signal mask to account for the removed signal.
	 *
	 */
	signalfd_remove(desc->signo);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	l_queue_destroy(desc->callbacks, NULL);
	l_free(desc);

done:
	if (signal->destroy)
		signal->destroy(signal->user_data);

	l_free(signal);
}
