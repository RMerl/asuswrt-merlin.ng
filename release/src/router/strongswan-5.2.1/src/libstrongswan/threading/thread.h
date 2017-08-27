/*
 * Copyright (C) 2009 Tobias Brunner
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

/**
 * @defgroup thread thread
 * @{ @ingroup threading
 */

#ifndef THREADING_THREAD_H_
#define THREADING_THREAD_H_

typedef struct thread_t thread_t;

#ifdef __APPLE__
/* thread_create is a syscall used to create Mach kernel threads and although
 * there are no errors or warnings during compilation or linkage the dynamic
 * linker does not use our implementation, therefore we rename it here
 */
#define thread_create(main, arg) strongswan_thread_create(main, arg)

/* on Mac OS X 10.5 several system calls we use are no cancellation points.
 * fortunately, select isn't one of them, so we wrap some of the others with
 * calls to select(2).
 */
#include <sys/socket.h>
#include <sys/select.h>

#define WRAP_WITH_SELECT(func, socket, ...)\
	fd_set rfds; FD_ZERO(&rfds); FD_SET(socket, &rfds);\
	if (select(socket + 1, &rfds, NULL, NULL, NULL) <= 0) { return -1; }\
	return func(socket, __VA_ARGS__)

static inline int cancellable_accept(int socket, struct sockaddr *address,
									 socklen_t *address_len)
{
	WRAP_WITH_SELECT(accept, socket, address, address_len);
}
#define accept cancellable_accept
static inline int cancellable_recvfrom(int socket, void *buffer, size_t length,
				int flags, struct sockaddr *address, socklen_t *address_len)
{
	WRAP_WITH_SELECT(recvfrom, socket, buffer, length, flags, address, address_len);
}
#define recvfrom cancellable_recvfrom
#endif /* __APPLE__ */

/**
 * Main function of a thread.
 *
 * @param arg			argument provided to constructor
 * @return				value provided to threads joining the terminating thread
 */
typedef void *(*thread_main_t)(void *arg);

/**
 * Cleanup callback function for a thread.
 *
 * @param arg			argument provided to thread_cleanup_push
 */
typedef void (*thread_cleanup_t)(void *arg);

/**
 * Thread wrapper implements simple, portable and advanced thread functions.
 *
 * @note All threads other than the main thread need either to be joined or
 * detached by calling the corresponding method.
 */
struct thread_t {

	/**
	 * Cancel this thread.
	 */
	void (*cancel)(thread_t *this);

	/**
	 * Send a signal to this thread.
	 *
	 * @param sig		the signal to be sent to this thread
	 */
	void (*kill)(thread_t *this, int sig);

	/**
	 * Detach this thread, this automatically destroys the thread object after
	 * the thread returned from its main function.
	 *
	 * @note Calling detach is like calling destroy on other objects.
	 */
	void (*detach)(thread_t *this);

	/**
	 * Join this thread, this automatically destroys the thread object
	 * afterwards.
	 *
	 * @note Calling join is like calling destroy on other objects.
	 *
	 * @return			the value returned from the thread's main function or
	 *					a call to exit.
	 */
	void *(*join)(thread_t *this);
};

/**
 * Create a new thread instance.
 *
 * @param main			thread main function
 * @param arg			argument provided to the main function
 * @return				thread instance
 */
thread_t *thread_create(thread_main_t main, void *arg);

/**
 * Get a thread object for the current thread.
 *
 * @return				thread instance
 */
thread_t *thread_current();

/**
 * Get the human-readable ID of the current thread.
 *
 * The IDs are assigned incrementally starting from 1.
 *
 * @return				human-readable ID
 */
u_int thread_current_id();

/**
 * Push a function onto the current thread's cleanup handler stack.
 * The callback function is called whenever the thread is cancelled, exits or
 * thread_cleanup_pop is called with TRUE as execute argument.
 *
 * @param cleanup		function called on thread exit
 * @param arg			argument provided to the callback
 */
void thread_cleanup_push(thread_cleanup_t cleanup, void *arg);

/**
 * Remove the top function from the current thread's cleanup handler stack
 * and optionally execute it.
 *
 * @param execute		TRUE to execute the function
 */
void thread_cleanup_pop(bool execute);

/**
 * Enable or disable the cancelability of the current thread. The current
 * value is returned.
 *
 * @param enable		TRUE to enable cancelability
 * @return				the current state of the cancelability
 */
bool thread_cancelability(bool enable);

/**
 * Force creation of a cancellation point in the calling thread.
 *
 * This temporarily enables thread cancelability, tests for a pending
 * cancellation request and then disables cancelability again if it was
 * disabled before the call to thread_cancellation_point().
 */
void thread_cancellation_point();

/**
 * Exit the current thread.
 *
 * @param val			value provided to threads joining the current thread
 */
void thread_exit(void *val);

/**
 * Called by the main thread to initialize the thread management.
 */
void threads_init();

/**
 * Called by the main thread to deinitialize the thread management.
 */
void threads_deinit();


#ifdef __APPLE__

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

#endif /* __APPLE__ */

#endif /** THREADING_THREAD_H_ @} */
