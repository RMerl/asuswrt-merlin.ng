/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "thread.h"

#include <utils/debug.h>
#include <threading/spinlock.h>
#include <threading/thread.h>
#include <collections/hashtable.h>
#include <collections/array.h>


typedef struct private_thread_t private_thread_t;

struct private_thread_t {

	/**
	 * Public interface.
	 */
	thread_t public;

	/**
	 * GetCurrentThreadId() of thread
	 */
	DWORD id;

	/**
	 * Printable thread id returned by thread_current_id()
	 */
	u_int tid;

	/**
	 * Windows thread handle
	 */
	HANDLE handle;

	/**
	 * Main function of this thread (NULL for the main thread).
	 */
	thread_main_t main;

	/**
	 * Argument for the main function.
	 */
	void *arg;

	/**
	 * Thread return value
	 */
	void *ret;

	/**
	 * Stack of cleanup handlers, as cleanup_t
	 */
	array_t *cleanup;

	/**
	 * Thread specific values for this thread
	 */
	hashtable_t *tls;

	/**
	 * Thread terminated?
	 */
	bool terminated;

	/**
	 * Thread detached?
	 */
	bool detached;

	/**
	 * Is thread in cancellable state
	 */
	bool cancelability;

	/**
	 * Has the thread been cancelled by thread->cancel()?
	 */
	bool canceled;

	/**
	 * Did we schedule an APC to docancel()?
	 */
	bool cancel_pending;

	/**
	 * Active condition variable thread is waiting in, if any
	 */
	CONDITION_VARIABLE *condvar;
};

/**
 * Global list of threads, GetCurrentThreadId() => private_thread_t
 */
static hashtable_t *threads;

/**
 * Lock for threads table
 */
static spinlock_t *threads_lock;

/**
 * Counter to assign printable thread IDs
 */
static u_int threads_ids = 0;

/**
 * Forward declaration
 */
static private_thread_t *create_internal(DWORD id);

/**
 * Set leak detective state
 */
static inline bool set_leak_detective(bool state)
{
#ifdef LEAK_DETECTIVE
	if (lib && lib->leak_detective)
	{
		return lib->leak_detective->set_state(lib->leak_detective, state);
	}
#endif
	return FALSE;
}

/**
 * Store thread in index
 */
static void put_thread(private_thread_t *this)
{
	bool old;

	old = set_leak_detective(FALSE);
	threads_lock->lock(threads_lock);

	threads->put(threads, (void*)(uintptr_t)this->id, this);

	threads_lock->unlock(threads_lock);
	set_leak_detective(old);
}

/**
 * Remove thread from index
 */
static void remove_thread(private_thread_t *this)
{
	bool old;

	old = set_leak_detective(FALSE);
	threads_lock->lock(threads_lock);

	threads->remove(threads, (void*)(uintptr_t)this->id);

	threads_lock->unlock(threads_lock);
	set_leak_detective(old);
}

/**
 * Get thread data for calling thread
 */
static private_thread_t *get_current_thread()
{
	private_thread_t *this;

	threads_lock->lock(threads_lock);

	this = threads->get(threads, (void*)(uintptr_t)GetCurrentThreadId());

	threads_lock->unlock(threads_lock);

	if (!this)
	{
		this = create_internal(GetCurrentThreadId());
		put_thread(this);
	}

	return this;
}

/**
 * See header.
 */
void* thread_tls_put(void *key, void *value)
{
	private_thread_t *thread;
	bool old;

	thread = get_current_thread();

	old = set_leak_detective(FALSE);
	value = thread->tls->put(thread->tls, key, value);
	set_leak_detective(old);

	return value;
}

/**
 * See header.
 */
void* thread_tls_get(void *key)
{
	private_thread_t *thread;
	void *value;
	bool old;

	thread = get_current_thread();

	old = set_leak_detective(FALSE);
	value = thread->tls->get(thread->tls, key);
	set_leak_detective(old);

	return value;
}

/**
 * See header.
 */
void* thread_tls_remove(void *key)
{
	private_thread_t *thread;
	void *value;
	bool old;

	thread = get_current_thread();

	old = set_leak_detective(FALSE);
	threads_lock->lock(threads_lock);
	value = thread->tls->remove(thread->tls, key);
	threads_lock->unlock(threads_lock);
	set_leak_detective(old);

	return value;
}

/**
 * Thread cleanup data
 */
typedef struct {
	/** Cleanup callback function */
	thread_cleanup_t cb;
	/** Argument provided to the cleanup function */
	void *arg;
} cleanup_t;

/**
 * Invoke pushed/tls cleanup handlers
 */
static void docleanup(private_thread_t *this)
{
	enumerator_t *enumerator;
	cleanup_t cleanup, *tls;
	bool old;

	old = set_leak_detective(FALSE);

	while (array_remove(this->cleanup, -1, &cleanup))
	{
		set_leak_detective(old);
		cleanup.cb(cleanup.arg);
		set_leak_detective(FALSE);
	}

	threads_lock->lock(threads_lock);
	enumerator = this->tls->create_enumerator(this->tls);
	while (enumerator->enumerate(enumerator, NULL, &tls))
	{
		this->tls->remove_at(this->tls, enumerator);

		set_leak_detective(old);
		thread_tls_cleanup(tls);
		set_leak_detective(FALSE);
	}
	enumerator->destroy(enumerator);
	threads_lock->unlock(threads_lock);

	set_leak_detective(old);
}

/**
 * Clean up and destroy a thread
 */
static void destroy(private_thread_t *this)
{
	bool old;

	docleanup(this);

	old = set_leak_detective(FALSE);

	array_destroy(this->cleanup);
	this->tls->destroy(this->tls);
	if (this->handle)
	{
		CloseHandle(this->handle);
	}
	free(this);

	set_leak_detective(old);
}

/**
 * End a thread, destroy when detached
 */
static void end_thread(private_thread_t *this)
{
	if (this->detached)
	{
		remove_thread(this);
		destroy(this);
	}
	else
	{
		this->terminated = TRUE;
		docleanup(this);
	}
}

/**
 * See header.
 */
void thread_set_active_condvar(CONDITION_VARIABLE *condvar)
{
	private_thread_t *thread;

	thread = get_current_thread();

	threads_lock->lock(threads_lock);
	thread->condvar = condvar;
	threads_lock->unlock(threads_lock);

	/* this is a cancellation point, as condvar wait is one */
	SleepEx(0, TRUE);
}

/**
 * APC to cancel a thread
 */
static void WINAPI docancel(ULONG_PTR dwParam)
{
	private_thread_t *this = (private_thread_t*)dwParam;

	/* make sure cancel() does not access this anymore */
	threads_lock->lock(threads_lock);
	threads_lock->unlock(threads_lock);

	end_thread(this);
	ExitThread(0);
}

METHOD(thread_t, cancel, void,
	private_thread_t *this)
{
	this->canceled = TRUE;
	if (this->cancelability)
	{
		threads_lock->lock(threads_lock);
		if (!this->cancel_pending)
		{
			this->cancel_pending = TRUE;
			QueueUserAPC(docancel, this->handle, (uintptr_t)this);
			if (this->condvar)
			{
				WakeAllConditionVariable(this->condvar);
			}
		}
		threads_lock->unlock(threads_lock);
	}
}

METHOD(thread_t, kill_, void,
	private_thread_t *this, int sig)
{
}

METHOD(thread_t, detach, void,
	private_thread_t *this)
{
	this->detached = TRUE;
}

METHOD(thread_t, join, void*,
	private_thread_t *this)
{
	void *ret;

	if (this->detached)
	{
		return NULL;
	}

	while (!this->terminated)
	{
		/* join is a cancellation point, use alertable wait */
		WaitForSingleObjectEx(this->handle, INFINITE, TRUE);
	}

	ret = this->ret;

	remove_thread(this);
	destroy(this);

	return ret;
}

/**
 * Main function wrapper for threads
 */
static DWORD thread_cb(private_thread_t *this)
{
	/* Enable cancelability once the thread starts. We must check for any
	 * pending cancellation request an queue the APC that gets executed
	 * at the first cancellation point. */
	this->cancelability = TRUE;
	if (this->canceled)
	{
		cancel(this);
	}

	this->ret = this->main(this->arg);

	end_thread(this);

	return 0;
}

/**
 * Create an internal thread object.
 */
static private_thread_t *create_internal(DWORD id)
{
	private_thread_t *this;
	bool old;

	old = set_leak_detective(FALSE);

	INIT(this,
		.public = {
			.cancel = _cancel,
			.kill = _kill_,
			.detach = _detach,
			.join = _join,
		},
		.cleanup = array_create(sizeof(cleanup_t), 0),
		.tls = hashtable_create(hashtable_hash_ptr, hashtable_equals_ptr, 4),
		.id = id,
		.cancelability = TRUE,
	);

	set_leak_detective(old);

	threads_lock->lock(threads_lock);
	this->tid = threads_ids++;
	threads_lock->unlock(threads_lock);

	if (id)
	{
		this->handle = OpenThread(THREAD_ALL_ACCESS, FALSE, id);
	}
	return this;
}

/**
 * Described in header.
 */
thread_t *thread_create(thread_main_t main, void *arg)
{
	private_thread_t *this;

	this = create_internal(0);

	this->main = main;
	this->arg = arg;
	/* not cancellable until started */
	this->cancelability = FALSE;

	this->handle = CreateThread(NULL, 0, (void*)thread_cb, this,
								CREATE_SUSPENDED, &this->id);
	if (!this->handle)
	{
		destroy(this);
		return NULL;
	}

	put_thread(this);

	DBG2(DBG_LIB, "created thread %u", this->id);

	ResumeThread(this->handle);

	return &this->public;
}

/**
 * Described in header.
 */
thread_t *thread_current()
{
	return &get_current_thread()->public;
}

/**
 * Described in header.
 */
u_int thread_current_id()
{
	return get_current_thread()->tid;
}

/**
 * Described in header.
 */
void thread_cleanup_push(thread_cleanup_t cb, void *arg)
{
	private_thread_t *this;
	cleanup_t cleanup = {
		.cb = cb,
		.arg = arg,
	};
	bool old;

	this = get_current_thread();

	old = set_leak_detective(FALSE);
	array_insert(this->cleanup, -1, &cleanup);
	set_leak_detective(old);
}

/**
 * Described in header
 */
void thread_cleanup_pop(bool execute)
{
	private_thread_t *this;
	cleanup_t cleanup = {};
	bool old;

	this = get_current_thread();

	old = set_leak_detective(FALSE);
	array_remove(this->cleanup, -1, &cleanup);
	set_leak_detective(old);

	if (execute)
	{
		cleanup.cb(cleanup.arg);
	}
}

/**
 * Described in header.
 */
bool thread_cancelability(bool enable)
{
	private_thread_t *this;
	bool old;

	this = get_current_thread();
	old = this->cancelability;
	this->cancelability = enable;

	if (enable && !old && this->canceled)
	{
		cancel(this);
	}
	return old;
}

/**
 * Described in header.
 */
void thread_cancellation_point()
{
	bool old;

	old = thread_cancelability(TRUE);
	SleepEx(0, TRUE);
	thread_cancelability(old);
}

/**
 * Described in header.
 */
void thread_exit(void *val)
{
	private_thread_t *this;

	this = get_current_thread();
	this->ret = val;

	end_thread(this);
	ExitThread(0);
}

/**
 * Clean up thread data while it detaches
 */
static void cleanup_tls()
{
	private_thread_t *this;
	bool old;

	old = set_leak_detective(FALSE);
	threads_lock->lock(threads_lock);

	this = threads->remove(threads, (void*)(uintptr_t)GetCurrentThreadId());

	threads_lock->unlock(threads_lock);
	set_leak_detective(old);

	if (this)
	{
		/* If the thread exited, but has not been joined, it is in terminated
		 * state. We must not mangle it, as we target externally spawned
		 * threads only. */
		if (!this->terminated && !this->detached)
		{
			destroy(this);
		}
	}
}

/**
 * DllMain called for dll events
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_THREAD_DETACH:
			cleanup_tls();
			break;
		default:
			break;
	}
	return TRUE;
}

/*
 * Described in header.
 */
void threads_init()
{
	threads_lock = spinlock_create();
	threads = hashtable_create(hashtable_hash_ptr, hashtable_equals_ptr, 4);

	/* reset counter should we initialize more than once */
	threads_ids = 0;

	put_thread(create_internal(GetCurrentThreadId()));
}

/**
 * Described in header.
 */
void threads_deinit()
{
	private_thread_t *this;

	this = threads->remove(threads, (void*)(uintptr_t)GetCurrentThreadId());
	destroy(this);

	threads_lock->destroy(threads_lock);
	threads->destroy(threads);
}
