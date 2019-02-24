/*
 * Copyright (C) 2009 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include <utils/utils.h>

typedef struct thread_t thread_t;

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
 * Get the ID of the current thread.
 *
 * Depending on the build configuration thread IDs are either assigned
 * incrementally starting from 1, or equal the value returned by an appropriate
 * syscall (like gettid() or GetCurrentThreadId()), if available.
 *
 * @return				ID of the current thread
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
 * Pop and execute all cleanup handlers in reverse order of registration.
 *
 * This function is for very special purposes only, where the caller exactly
 * knows which cleanup handlers have been pushed. For regular use, a caller
 * should thread_cleanup_pop() exactly the number of handlers it pushed
 * using thread_cleanup_push().
 */
void thread_cleanup_popall();

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

#endif /** THREADING_THREAD_H_ @} */
