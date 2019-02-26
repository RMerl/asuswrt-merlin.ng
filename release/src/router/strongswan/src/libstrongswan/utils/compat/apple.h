/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup apple apple
 * @{ @ingroup compat
 */

#ifndef APPLE_H_
#define APPLE_H_

#include <poll.h>
#include <sys/socket.h>
#include <sys/select.h>

/* thread_create is a syscall used to create Mach kernel threads and although
 * there are no errors or warnings during compilation or linkage the dynamic
 * linker does not use our implementation, therefore we rename it here
 */
#define thread_create(main, arg) strongswan_thread_create(main, arg)

/* Mach uses a semaphore_create() call, use a different name for ours */
#define semaphore_create(x) strongswan_semaphore_create(x)

/* Since OS X 10.10 XPC includes some additional conflicting Mach types */
#define host_t strongswan_host_t
#define processor_t strongswan_processor_t
#define task_t strongswan_task_t
#define thread_t strongswan_thread_t

/* forward declaration, see below */
static inline int precancellable_poll(struct pollfd fds[], nfds_t nfds,
									  int timeout);

/* on Mac OS X 10.5 several system calls we use are no cancellation points.
 * fortunately, select isn't one of them, so we wrap some of the others with
 * calls to select(2).
 */

#define WRAP_WITH_POLL(func, socket, ...) \
	struct pollfd pfd = { \
		.fd = socket, \
		.events = POLLIN, \
	}; \
	if (precancellable_poll(&pfd, 1, -1) <= 0) \
	{\
		return -1; \
	}\
	return func(socket, __VA_ARGS__)

static inline int cancellable_accept(int socket, struct sockaddr *address,
									 socklen_t *address_len)
{
	WRAP_WITH_POLL(accept, socket, address, address_len);
}
#define accept cancellable_accept
static inline int cancellable_recvfrom(int socket, void *buffer, size_t length,
				int flags, struct sockaddr *address, socklen_t *address_len)
{
	WRAP_WITH_POLL(recvfrom, socket, buffer, length, flags, address, address_len);
}
#define recvfrom cancellable_recvfrom

#include <threading/thread.h>

/*
 * While select() is a cancellation point, it seems that OS X does not honor
 * pending cancellation points when entering the function. We manually test for
 * and honor pending cancellation requests, but this obviously can't prevent
 * some race conditions where the the cancellation happens after the check,
 * but before the select.
 */
static inline int precancellable_select(int nfds, fd_set *restrict readfds,
						fd_set *restrict writefds, fd_set *restrict errorfds,
						struct timeval *restrict timeout)
{
	if (thread_cancelability(TRUE))
	{
		thread_cancellation_point();
	}
	else
	{
		thread_cancelability(FALSE);
	}
	return select(nfds, readfds, writefds, errorfds, timeout);
}
#define select precancellable_select

/*
 * The same as to select(2) applies to poll(2)
 */
static inline int precancellable_poll(struct pollfd fds[], nfds_t nfds,
									  int timeout)
{
	if (thread_cancelability(TRUE))
	{
		thread_cancellation_point();
	}
	else
	{
		thread_cancelability(FALSE);
	}
	return poll(fds, nfds, timeout);
}
#define poll precancellable_poll

#endif /** APPLE_H_ @}*/
