/*
 * Copyright (C) 2016-2023 Tobias Brunner
 * Copyright (C) 2013 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include "watcher.h"

#include <library.h>
#include <threading/thread.h>
#include <threading/mutex.h>
#include <threading/condvar.h>
#include <collections/linked_list.h>
#include <processing/jobs/callback_job.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

typedef struct private_watcher_t private_watcher_t;
typedef struct entry_t entry_t;

/**
 * Private data of an watcher_t object.
 */
struct private_watcher_t {

	/**
	 * Public watcher_t interface.
	 */
	watcher_t public;

	/**
	 * List of registered FDs
	 */
	entry_t *fds;

	/**
	 * Last registered FD
	 */
	entry_t *last;

	/**
	 * Number of registered FDs
	 */
	u_int count;

	/**
	 * Pending update of FD list?
	 */
	bool pending;

	/**
	 * Running state of watcher
	 */
	watcher_state_t state;

	/**
	 * Lock to access FD list
	 */
	mutex_t *mutex;

	/**
	 * Condvar to signal completion of callback
	 */
	condvar_t *condvar;

	/**
	 * Notification pipe to signal watcher thread
	 */
	int notify[2];

	/**
	 * List of callback jobs to process by watcher thread, as job_t
	 */
	linked_list_t *jobs;
};

/**
 * Entry for a registered file descriptor
 */
struct entry_t {
	/** file descriptor */
	int fd;
	/** events to watch */
	watcher_event_t events;
	/** registered callback function */
	watcher_cb_t cb;
	/** user data to pass to callback */
	void *data;
	/** callback(s) currently active? */
	int in_callback;
	/** next registered fd */
	entry_t *next;
};

/**
 * Adds the given entry at the end of the list
 */
static void add_entry(private_watcher_t *this, entry_t *entry)
{
	if (this->last)
	{
		this->last->next = entry;
		this->last = entry;
	}
	else
	{
		this->fds = this->last = entry;
	}
	this->count++;
}

/**
 * Removes and frees the given entry
 *
 * Updates the previous entry and returns the next entry in the list, if any.
 */
static entry_t *remove_entry(private_watcher_t *this, entry_t *entry,
							 entry_t *prev)
{
	entry_t *next = entry->next;

	if (prev)
	{
		prev->next = next;
	}
	else
	{
		this->fds = next;
	}
	if (this->last == entry)
	{
		this->last = prev;
	}
	this->count--;
	free(entry);
	return next;
}

/**
 * Data we pass on for an async notification
 */
typedef struct {
	/** triggering entry */
	entry_t *entry;
	/** file descriptor */
	int fd;
	/** event type */
	watcher_event_t event;
	/** registered callback function */
	watcher_cb_t cb;
	/** user data to pass to callback */
	void *data;
	/** keep registered? */
	bool keep;
	/** reference to watcher */
	private_watcher_t *this;
} notify_data_t;

/**
 * Notify watcher thread about changes and unlock mutex
 */
static void update_and_unlock(private_watcher_t *this)
{
	char buf[1] = { 'u' };
	int error = 0;

	this->pending = TRUE;
	if (this->notify[1] != -1)
	{
		if (write(this->notify[1], buf, sizeof(buf)) == -1)
		{
			error = errno;
		}
	}
	this->mutex->unlock(this->mutex);

	if (error)
	{
		DBG1(DBG_JOB, "notifying watcher failed: %s", strerror(error));
	}
}

/**
 * Cleanup function if callback gets canceled
 */
static void unregister(notify_data_t *data)
{
	/* if a thread processing a callback gets canceled, we mark the entry
	 * as canceled, like the callback would return FALSE. This is required
	 * to not queue this watcher again if all threads have been gone. */
	data->keep = FALSE;
}

 /**
 * Execute callback of registered FD, asynchronous
 */
static job_requeue_t notify_async(notify_data_t *data)
{
	thread_cleanup_push((void*)unregister, data);
	data->keep = data->cb(data->data, data->fd, data->event);
	thread_cleanup_pop(FALSE);
	return JOB_REQUEUE_NONE;
}

/**
 * Clean up notification data, reactivate FD
 */
static void notify_end(notify_data_t *data)
{
	private_watcher_t *this = data->this;
	entry_t *entry, *prev = NULL;
	watcher_event_t updated = 0;
	bool removed = FALSE;

	/* reactivate the disabled entry */
	this->mutex->lock(this->mutex);
	for (entry = this->fds; entry; prev = entry, entry = entry->next)
	{
		if (entry == data->entry)
		{
			if (!data->keep)
			{
				entry->events &= ~data->event;
				updated = entry->events;
				if (!entry->events)
				{
					remove_entry(this, entry, prev);
					removed = TRUE;
					break;
				}
			}
			entry->in_callback--;
			break;
		}
	}
	this->condvar->broadcast(this->condvar);
	update_and_unlock(this);

	if (removed)
	{
		DBG3(DBG_JOB, "removed fd %d[%s%s] from watcher after callback", data->fd,
			 data->event & WATCHER_READ ? "r" : "",
			 data->event & WATCHER_WRITE ? "w" : "");
	}
	else if (updated)
	{
		DBG3(DBG_JOB, "updated fd %d[%s%s] to %d[%s%s] after callback", data->fd,
			 (updated | data->event) & WATCHER_READ ? "r" : "",
			 (updated | data->event) & WATCHER_WRITE ? "w" : "", data->fd,
			 updated & WATCHER_READ ? "r" : "",
			 updated & WATCHER_WRITE ? "w" : "");
	}
	free(data);
}

/**
 * Execute the callback for a registered FD
 */
static void notify(private_watcher_t *this, entry_t *entry,
				   watcher_event_t event)
{
	notify_data_t *data;

	/* get a copy of entry for async job, but with specific event */
	INIT(data,
		.entry = entry,
		.fd = entry->fd,
		.event = event,
		.cb = entry->cb,
		.data = entry->data,
		.keep = TRUE,
		.this = this,
	);

	/* deactivate entry, so we can poll() other FDs even if the async
	 * processing did not handle the event yet */
	entry->in_callback++;

	this->jobs->insert_last(this->jobs,
					callback_job_create_with_prio((void*)notify_async, data,
						(void*)notify_end, (callback_job_cancel_t)return_false,
						JOB_PRIO_CRITICAL));
}

/**
 * Thread cancellation function for watcher thread
 */
static void activate_all(private_watcher_t *this)
{
	entry_t *entry;

	/* When the watcher thread gets canceled, we have to reactivate any entry
	 * and signal threads in remove() to go on. */

	this->mutex->lock(this->mutex);
	for (entry = this->fds; entry; entry = entry->next)
	{
		entry->in_callback = 0;
	}
	this->state = WATCHER_STOPPED;
	this->condvar->broadcast(this->condvar);
	this->mutex->unlock(this->mutex);
}

/**
 * Find flagged revents in a pollfd set by fd
 */
static inline int find_revents(struct pollfd *pfd, int count, int fd)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (pfd[i].fd == fd)
		{
			return pfd[i].revents;
		}
	}
	return 0;
}

#if DEBUG_LEVEL >= 2
#define reset_log(buf, pos, len) ({ buf[0] = '\0'; pos = buf; len = sizeof(buf); })
#define reset_event_log(buf, pos) ({ pos = buf; })
#define end_event_log(pos) ({ *pos = '\0'; })
#define log_event(pos, ev) ({ *pos++ = ev; })
#define log_fd(pos, len, fd, ev) ({ \
	if (ev[0]) \
	{ \
		int _add = snprintf(pos, len, " %d[%s]", fd, ev); \
		if (_add >= 0 && _add < len) \
		{ \
			pos += _add; \
			len -= _add; \
		} \
	} \
})
#else
#define reset_event_log(...) ({})
#define end_event_log(...) ({})
#define log_event(...) ({})
#define reset_log(...) ({})
#define log_fd(...) ({})
#endif

/**
 * Dispatching function
 */
static job_requeue_t watch(private_watcher_t *this)
{
	entry_t *entry;
	struct pollfd *pfd;
	int count = 0, res;
#if DEBUG_LEVEL >= 2
	char logbuf[BUF_LEN], *logpos, eventbuf[4], *eventpos;
	int loglen;
#endif

	reset_log(logbuf, logpos, loglen);

	this->mutex->lock(this->mutex);

	count = this->count;
	if (!count)
	{
		this->state = WATCHER_STOPPED;
		this->mutex->unlock(this->mutex);
		return JOB_REQUEUE_NONE;
	}
	if (this->state == WATCHER_QUEUED)
	{
		this->state = WATCHER_RUNNING;
	}

	pfd = alloca(sizeof(*pfd) * (count + 1));
	pfd[0].fd = this->notify[0];
	pfd[0].events = POLLIN;
	count = 1;

	for (entry = this->fds; entry; entry = entry->next)
	{
		if (!entry->in_callback)
		{
			pfd[count].fd = entry->fd;
			pfd[count].events = 0;
			reset_event_log(eventbuf, eventpos);
			if (entry->events & WATCHER_READ)
			{
				log_event(eventpos, 'r');
				pfd[count].events |= POLLIN;
			}
			if (entry->events & WATCHER_WRITE)
			{
				log_event(eventpos, 'w');
				pfd[count].events |= POLLOUT;
			}
			end_event_log(eventpos);
			log_fd(logpos, loglen, entry->fd, eventbuf);
			count++;
		}
	}
	this->mutex->unlock(this->mutex);

#if DEBUG_LEVEL >= 3
	if (logbuf[0])
	{
		DBG3(DBG_JOB, "observing fds:%s", logbuf);
	}
#endif

	while (TRUE)
	{
		int revents;
		char buf[1];
		bool old;
		ssize_t len;
		job_t *job;

		DBG2(DBG_JOB, "watcher is observing %d fds", count-1);
		thread_cleanup_push((void*)activate_all, this);
		old = thread_cancelability(TRUE);

		res = poll(pfd, count, -1);
		if (res == -1 && errno == EINTR)
		{
			/* LinuxThreads interrupts poll(), but does not make it a
			 * cancellation point. Manually test if we got canceled. */
			thread_cancellation_point();
		}

		thread_cancelability(old);
		thread_cleanup_pop(FALSE);

		if (res > 0)
		{
			if (pfd[0].revents & POLLIN)
			{
				while (TRUE)
				{
					len = read(this->notify[0], buf, sizeof(buf));
					if (len == -1)
					{
						if (errno != EAGAIN && errno != EWOULDBLOCK)
						{
							DBG1(DBG_JOB, "reading watcher notify failed: %s",
								 strerror(errno));
						}
						break;
					}
				}
				this->pending = FALSE;
				DBG2(DBG_JOB, "watcher got notification, rebuilding");
				break;
			}

			reset_log(logbuf, logpos, loglen);
			this->mutex->lock(this->mutex);
			for (entry = this->fds; entry; entry = entry->next)
			{
				if (entry->in_callback)
				{
					continue;
				}
				reset_event_log(eventbuf, eventpos);
				revents = find_revents(pfd, count, entry->fd);
				if (revents & POLLERR)
				{
					log_event(eventpos, 'e');
				}
				if (revents & POLLIN)
				{
					log_event(eventpos, 'r');
				}
				if (revents & POLLOUT)
				{
					log_event(eventpos, 'w');
				}
				if (entry->events & WATCHER_READ &&
					revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL))
				{
					notify(this, entry, WATCHER_READ);
				}
				if (entry->events & WATCHER_WRITE &&
					revents & (POLLOUT | POLLERR | POLLHUP | POLLNVAL))
				{
					notify(this, entry, WATCHER_WRITE);
				}
				end_event_log(eventpos);
				log_fd(logpos, loglen, entry->fd, eventbuf);
			}
			this->mutex->unlock(this->mutex);

#if DEBUG_LEVEL >= 2
			if (logbuf[0])
			{
				DBG2(DBG_JOB, "events on fds:%s", logbuf);
			}
#endif

			if (this->jobs->get_count(this->jobs))
			{
				while (this->jobs->remove_first(this->jobs,
												(void**)&job) == SUCCESS)
				{
					lib->processor->execute_job(lib->processor, job);
				}
				/* we temporarily disable a notified FD, rebuild FDSET */
				break;
			}
		}
		else
		{
			if (!this->pending && errno != EINTR)
			{	/* complain only if no pending updates */
				DBG1(DBG_JOB, "watcher poll() error: %s", strerror(errno));
			}
			break;
		}
	}
	return JOB_REQUEUE_DIRECT;
}

METHOD(watcher_t, add, void,
	private_watcher_t *this, int fd, watcher_event_t events,
	watcher_cb_t cb, void *data)
{
	entry_t *entry;

	INIT(entry,
		.fd = fd,
		.events = events,
		.cb = cb,
		.data = data,
	);

	DBG3(DBG_JOB, "adding fd %d[%s%s] to watcher", fd,
		 events & WATCHER_READ ? "r" : "",
		 events & WATCHER_WRITE ? "w" : "");

	this->mutex->lock(this->mutex);
	add_entry(this, entry);
	if (this->state == WATCHER_STOPPED)
	{
		this->state = WATCHER_QUEUED;
		this->mutex->unlock(this->mutex);

		lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create_with_prio((void*)watch, this,
				NULL, (callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL));
	}
	else
	{
		update_and_unlock(this);
	}
}

METHOD(watcher_t, remove_, void,
	private_watcher_t *this, int fd)
{
	entry_t *entry, *prev = NULL;
	watcher_event_t found = 0;

	this->mutex->lock(this->mutex);
	while (TRUE)
	{
		bool is_in_callback = FALSE;

		entry = this->fds;
		while (entry)
		{
			if (entry->fd == fd)
			{
				if (this->state != WATCHER_STOPPED && entry->in_callback)
				{
					is_in_callback = TRUE;
					break;
				}
				found |= entry->events;
				entry = remove_entry(this, entry, prev);
				continue;
			}
			prev = entry;
			entry = entry->next;
		}
		if (!is_in_callback)
		{
			break;
		}
		this->condvar->wait(this->condvar, this->mutex);
	}
	if (found)
	{
		update_and_unlock(this);

		DBG3(DBG_JOB, "removed fd %d[%s%s] from watcher", fd,
			 found & WATCHER_READ ? "r" : "",
			 found & WATCHER_WRITE ? "w" : "");
	}
	else
	{
		this->mutex->unlock(this->mutex);
	}
}

METHOD(watcher_t, get_state, watcher_state_t,
	private_watcher_t *this)
{
	watcher_state_t state;

	this->mutex->lock(this->mutex);
	state = this->state;
	this->mutex->unlock(this->mutex);

	return state;
}

METHOD(watcher_t, destroy, void,
	private_watcher_t *this)
{
	this->mutex->destroy(this->mutex);
	this->condvar->destroy(this->condvar);
	if (this->notify[0] != -1)
	{
		close(this->notify[0]);
	}
	if (this->notify[1] != -1)
	{
		close(this->notify[1]);
	}
	this->jobs->destroy(this->jobs);
	free(this);
}

#ifdef WIN32

/**
 * Create notify pipe with a TCP socketpair
 */
static bool create_notify(private_watcher_t *this)
{
	u_long on = 1;

	if (socketpair(AF_INET, SOCK_STREAM, 0, this->notify) == 0)
	{
		/* use non-blocking I/O on read-end of notify pipe */
		if (ioctlsocket(this->notify[0], FIONBIO, &on) == 0)
		{
			return TRUE;
		}
		DBG1(DBG_LIB, "setting watcher notify pipe read-end non-blocking "
			 "failed: %s", strerror(errno));
	}
	return FALSE;
}

#else /* !WIN32 */

/**
 * Create a notify pipe with a one-directional pipe
 */
static bool create_notify(private_watcher_t *this)
{
	int flags;

	if (pipe(this->notify) == 0)
	{
		/* use non-blocking I/O on read-end of notify pipe */
		flags = fcntl(this->notify[0], F_GETFL);
		if (flags != -1 &&
			fcntl(this->notify[0], F_SETFL, flags | O_NONBLOCK) != -1)
		{
			return TRUE;
		}
		DBG1(DBG_LIB, "setting watcher notify pipe read-end non-blocking "
			 "failed: %s", strerror(errno));
	}
	return FALSE;
}

#endif /* !WIN32 */

/**
 * See header
 */
watcher_t *watcher_create()
{
	private_watcher_t *this;

	INIT(this,
		.public = {
			.add = _add,
			.remove = _remove_,
			.get_state = _get_state,
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
		.jobs = linked_list_create(),
		.notify = {-1, -1},
		.state = WATCHER_STOPPED,
	);

	if (!create_notify(this))
	{
		DBG1(DBG_LIB, "creating watcher notify pipe failed: %s",
			 strerror(errno));
	}
	return &this->public;
}
