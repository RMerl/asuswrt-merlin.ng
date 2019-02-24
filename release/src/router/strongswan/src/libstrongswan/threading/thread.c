/*
 * Copyright (C) 2009-2012 Tobias Brunner
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

#define _GNU_SOURCE
#include <pthread.h>
#include <signal.h>

#ifdef HAVE_GETTID
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef HAVE_SYS_GETTID
#include <sys/syscall.h>
static inline pid_t gettid()
{
	return syscall(SYS_gettid);
}
#endif

#include <library.h>
#include <utils/debug.h>

#include <threading/thread_value.h>
#include <threading/mutex.h>
#include <collections/linked_list.h>

#include "thread.h"

typedef struct private_thread_t private_thread_t;

struct private_thread_t {
	/**
	 * Public interface.
	 */
	thread_t public;

	/**
	 * Identificator of this thread (human-readable/thread ID).
	 */
	u_int id;

	/**
	 * ID of the underlying thread.
	 */
	pthread_t thread_id;

	/**
	 * Main function of this thread (NULL for the main thread).
	 */
	thread_main_t main;

	/**
	 * Argument for the main function.
	 */
	void *arg;

	/**
	 * Stack of cleanup handlers.
	 */
	linked_list_t *cleanup_handlers;

	/**
	 * Mutex to make modifying thread properties safe.
	 */
	mutex_t *mutex;

	/**
	 * TRUE if this thread has been detached or joined, i.e. can be cleaned
	 * up after terminating.
	 */
	bool detached_or_joined;

	/**
	 * TRUE if the threads has terminated (cancelled, via thread_exit or
	 * returned from the main function)
	 */
	bool terminated;

};

typedef struct {
	/**
	 * Cleanup callback function.
	 */
	thread_cleanup_t cleanup;

	/**
	 * Argument provided to the cleanup function.
	 */
	void *arg;

} cleanup_handler_t;


/**
 * Next thread ID.
 */
static u_int next_id;

/**
 * Mutex to safely access the next thread ID.
 */
static mutex_t *id_mutex;

/**
 * Store the thread object in a thread-specific value.
 */
static thread_value_t *current_thread;


#ifndef HAVE_PTHREAD_CANCEL
/* if pthread_cancel is not available, we emulate it using a signal */
#ifdef ANDROID
#define SIG_CANCEL SIGUSR2
#else
#define SIG_CANCEL (SIGRTMIN+7)
#endif

/* the signal handler for SIG_CANCEL uses pthread_exit to terminate the
 * "cancelled" thread */
static void cancel_signal_handler(int sig)
{
	pthread_exit(NULL);
}
#endif


/**
 * Destroy an internal thread object.
 *
 * @note The mutex of this thread object has to be locked, it gets unlocked
 * automatically.
 */
static void thread_destroy(private_thread_t *this)
{
	if (!this->terminated || !this->detached_or_joined)
	{
		this->mutex->unlock(this->mutex);
		return;
	}
	this->cleanup_handlers->destroy(this->cleanup_handlers);
	this->mutex->unlock(this->mutex);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * Determine the ID of the current thread
 */
static u_int get_thread_id()
{
	u_int id;

#if defined(USE_THREAD_IDS) && defined(HAVE_GETTID)
	id = gettid();
#else
	id_mutex->lock(id_mutex);
	id = next_id++;
	id_mutex->unlock(id_mutex);
#endif
	return id;
}

METHOD(thread_t, cancel, void,
	private_thread_t *this)
{
	this->mutex->lock(this->mutex);
	if (pthread_equal(this->thread_id, pthread_self()))
	{
		this->mutex->unlock(this->mutex);
		DBG1(DBG_LIB, "!!! CANNOT CANCEL CURRENT THREAD !!!");
		return;
	}
#ifdef HAVE_PTHREAD_CANCEL
	pthread_cancel(this->thread_id);
#else
	pthread_kill(this->thread_id, SIG_CANCEL);
#endif /* HAVE_PTHREAD_CANCEL */
	this->mutex->unlock(this->mutex);
}

METHOD(thread_t, kill_, void,
	private_thread_t *this, int sig)
{
	this->mutex->lock(this->mutex);
	if (pthread_equal(this->thread_id, pthread_self()))
	{
		/* it might actually be possible to send a signal to pthread_self (there
		 * is an example in raise(3) describing that), the problem is though,
		 * that the thread only returns here after the signal handler has
		 * returned, so depending on the signal, the lock might not get
		 * unlocked. */
		this->mutex->unlock(this->mutex);
		DBG1(DBG_LIB, "!!! CANNOT SEND SIGNAL TO CURRENT THREAD !!!");
		return;
	}
	pthread_kill(this->thread_id, sig);
	this->mutex->unlock(this->mutex);
}

METHOD(thread_t, detach, void,
	private_thread_t *this)
{
	this->mutex->lock(this->mutex);
	pthread_detach(this->thread_id);
	this->detached_or_joined = TRUE;
	thread_destroy(this);
}

METHOD(thread_t, join, void*,
	private_thread_t *this)
{
	pthread_t thread_id;
	void *val;

	this->mutex->lock(this->mutex);
	if (pthread_equal(this->thread_id, pthread_self()))
	{
		this->mutex->unlock(this->mutex);
		DBG1(DBG_LIB, "!!! CANNOT JOIN CURRENT THREAD !!!");
		return NULL;
	}
	if (this->detached_or_joined)
	{
		this->mutex->unlock(this->mutex);
		DBG1(DBG_LIB, "!!! CANNOT JOIN DETACHED THREAD !!!");
		return NULL;
	}
	thread_id = this->thread_id;
	this->detached_or_joined = TRUE;
	if (this->terminated)
	{
		/* thread has terminated before the call to join */
		thread_destroy(this);
	}
	else
	{
		/* thread_destroy is called when the thread terminates normally */
		this->mutex->unlock(this->mutex);
	}
	pthread_join(thread_id, &val);

	return val;
}

/**
 * Create an internal thread object.
 */
static private_thread_t *thread_create_internal()
{
	private_thread_t *this;

	INIT(this,
		.public = {
			.cancel = _cancel,
			.kill = _kill_,
			.detach = _detach,
			.join = _join,
		},
		.cleanup_handlers = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	return this;
}

/**
 * Remove and run all cleanup handlers in reverse order.
 */
static void thread_cleanup_popall_internal(private_thread_t *this)
{
	cleanup_handler_t *handler;

	while (this->cleanup_handlers->remove_last(this->cleanup_handlers,
											  (void**)&handler) == SUCCESS)
	{
		handler->cleanup(handler->arg);
		free(handler);
	}
}

/**
 * Main cleanup function for threads.
 */
static void thread_cleanup(private_thread_t *this)
{
	thread_cleanup_popall_internal(this);
	this->mutex->lock(this->mutex);
	this->terminated = TRUE;
	thread_destroy(this);
}

/**
 * Main function wrapper for threads.
 */
static void *thread_main(private_thread_t *this)
{
	void *res;

	this->id = get_thread_id();

	current_thread->set(current_thread, this);
	pthread_cleanup_push((thread_cleanup_t)thread_cleanup, this);

	/* TODO: this is not 100% portable as pthread_t is an opaque type (i.e.
	 * could be of any size, or even a struct) */
#ifdef HAVE_GETTID
	DBG2(DBG_LIB, "created thread %.2d [%u]",
		 this->id, gettid());
#elif defined(WIN32)
	DBG2(DBG_LIB, "created thread %.2d [%p]",
		 this->id, this->thread_id.p);
#else
	DBG2(DBG_LIB, "created thread %.2d [%lx]",
		 this->id, (u_long)this->thread_id);
#endif

	res = this->main(this->arg);
	pthread_cleanup_pop(TRUE);

	return res;
}

/**
 * Described in header.
 */
thread_t *thread_create(thread_main_t main, void *arg)
{
	private_thread_t *this = thread_create_internal();

	this->main = main;
	this->arg = arg;

	if (pthread_create(&this->thread_id, NULL, (void*)thread_main, this) != 0)
	{
		DBG1(DBG_LIB, "failed to create thread!");
		this->mutex->lock(this->mutex);
		this->terminated = TRUE;
		this->detached_or_joined = TRUE;
		thread_destroy(this);
		return NULL;
	}

	return &this->public;
}

/**
 * Described in header.
 */
thread_t *thread_current()
{
	private_thread_t *this;

	this = (private_thread_t*)current_thread->get(current_thread);
	if (!this)
	{
		this = thread_create_internal();
		this->id = get_thread_id();
		current_thread->set(current_thread, (void*)this);
	}
	return &this->public;
}

/**
 * Described in header.
 */
u_int thread_current_id()
{
	private_thread_t *this = (private_thread_t*)thread_current();

	return this ? this->id : 0;
}

/**
 * Described in header.
 */
void thread_cleanup_push(thread_cleanup_t cleanup, void *arg)
{
	private_thread_t *this = (private_thread_t*)thread_current();
	cleanup_handler_t *handler;

	INIT(handler,
		.cleanup = cleanup,
		.arg = arg,
	);

	this->cleanup_handlers->insert_last(this->cleanup_handlers, handler);
}

/**
 * Described in header.
 */
void thread_cleanup_pop(bool execute)
{
	private_thread_t *this = (private_thread_t*)thread_current();
	cleanup_handler_t *handler;

	if (this->cleanup_handlers->remove_last(this->cleanup_handlers,
											(void**)&handler) != SUCCESS)
	{
		DBG1(DBG_LIB, "!!! THREAD CLEANUP ERROR !!!");
		return;
	}

	if (execute)
	{
		handler->cleanup(handler->arg);
	}
	free(handler);
}

/**
 * Described in header.
 */
void thread_cleanup_popall()
{
	private_thread_t *this = (private_thread_t*)thread_current();

	thread_cleanup_popall_internal(this);
}

/**
 * Described in header.
 */
bool thread_cancelability(bool enable)
{
#ifdef HAVE_PTHREAD_CANCEL
	int old;

	pthread_setcancelstate(enable ? PTHREAD_CANCEL_ENABLE
								  : PTHREAD_CANCEL_DISABLE, &old);

	return old == PTHREAD_CANCEL_ENABLE;
#else
	sigset_t new, old;

	sigemptyset(&new);
	sigaddset(&new, SIG_CANCEL);
	pthread_sigmask(enable ? SIG_UNBLOCK : SIG_BLOCK, &new, &old);

	return sigismember(&old, SIG_CANCEL) == 0;
#endif /* HAVE_PTHREAD_CANCEL */
}

/**
 * Described in header.
 */
void thread_cancellation_point()
{
	bool old = thread_cancelability(TRUE);

#ifdef HAVE_PTHREAD_CANCEL
	pthread_testcancel();
#endif /* HAVE_PTHREAD_CANCEL */
	thread_cancelability(old);
}

/**
 * Described in header.
 */
void thread_exit(void *val)
{
	pthread_exit(val);
}

/**
 * A dummy thread value that reserved pthread_key_t value "0". A buggy PKCS#11
 * library mangles this key, without owning it, so we allocate it for them.
 */
static thread_value_t *dummy1;

/**
 * Described in header.
 */
void threads_init()
{
	private_thread_t *main_thread = thread_create_internal();

	dummy1 = thread_value_create(NULL);

	next_id = 0;
	main_thread->thread_id = pthread_self();
	current_thread = thread_value_create(NULL);
	current_thread->set(current_thread, (void*)main_thread);
	id_mutex = mutex_create(MUTEX_TYPE_DEFAULT);
	main_thread->id = get_thread_id();

#ifndef HAVE_PTHREAD_CANCEL
	{	/* install a signal handler for our custom SIG_CANCEL */
		struct sigaction action = {
			.sa_handler = cancel_signal_handler
		};
		sigaction(SIG_CANCEL, &action, NULL);
	}
#endif /* HAVE_PTHREAD_CANCEL */
}

/**
 * Described in header.
 */
void threads_deinit()
{
	private_thread_t *main_thread = (private_thread_t*)thread_current();

	dummy1->destroy(dummy1);

	main_thread->mutex->lock(main_thread->mutex);
	main_thread->terminated = TRUE;
	main_thread->detached_or_joined = TRUE;
	thread_destroy(main_thread);
	current_thread->destroy(current_thread);
	id_mutex->destroy(id_mutex);
}
