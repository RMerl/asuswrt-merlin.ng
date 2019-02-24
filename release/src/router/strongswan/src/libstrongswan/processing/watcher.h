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

/**
 * @defgroup watcher watcher
 * @{ @ingroup processor
 */

#ifndef WATCHER_H_
#define WATCHER_H_

typedef struct watcher_t watcher_t;
typedef enum watcher_event_t watcher_event_t;
typedef enum watcher_state_t watcher_state_t;

#include <library.h>

/**
 * Callback function to register for file descriptor events.
 *
 * The callback is executed asynchronously using a thread from the pool.
 * Monitoring of fd is temporarily suspended to avoid additional events while
 * it is processed asynchronously. To allow concurrent events, one can quickly
 * process it (using a read/write) and return from the callback. This will
 * re-enable the event, while the data read can be processed in another
 * asynchronous job.
 *
 * On Linux, even if select() marks an FD as "ready", a subsequent read/write
 * can block. It is therefore highly recommended to use non-blocking I/O
 * and handle EAGAIN/EWOULDBLOCK gracefully.
 *
 * @param data		user data passed during registration
 * @param fd		file descriptor the event occurred on
 * @param event		type of event
 * @return			TRUE to keep watching event, FALSE to unregister fd for event
 */
typedef bool (*watcher_cb_t)(void *data, int fd, watcher_event_t event);

/**
 * What events to watch for a file descriptor.
 */
enum watcher_event_t {
	WATCHER_READ = (1<<0),
	WATCHER_WRITE = (1<<1),
	WATCHER_EXCEPT = (1<<2),
};

/**
 * State the watcher currently is in
 */
enum watcher_state_t {
	/** no watcher thread running or queued */
	WATCHER_STOPPED = 0,
	/** a job has been queued for watching, but not yet started */
	WATCHER_QUEUED,
	/** a watcher thread is active, dispatching socket events */
	WATCHER_RUNNING,
};

/**
 * Watch multiple file descriptors using select().
 */
struct watcher_t {

	/**
	 * Start watching a new file descriptor.
	 *
	 * Multiple callbacks can be registered for the same file descriptor, and
	 * all of them get notified. Such callbacks are executed concurrently.
	 *
	 * @param fd		file descriptor to start watching
	 * @param events	ORed set of events to watch
	 * @param cb		callback function to invoke on events
	 * @param data		data to pass to cb()
	 */
	void (*add)(watcher_t *this, int fd, watcher_event_t events,
				watcher_cb_t cb, void *data);

	/**
	 * Stop watching a previously registered file descriptor.
	 *
	 * This call blocks until any active callback for this FD returns. All
	 * callbacks registered for that FD get unregistered.
	 *
	 * @param fd		file descriptor to stop watching
	 */
	void (*remove)(watcher_t *this, int fd);

	/**
	 * Get the current watcher state
	 *
	 * @return			currently active watcher state
	 */
	watcher_state_t (*get_state)(watcher_t *this);

	/**
	 * Destroy a watcher_t.
	 */
	void (*destroy)(watcher_t *this);
};

/**
 * Create a watcher instance.
 *
 * @return		watcher
 */
watcher_t *watcher_create();

#endif /** WATCHER_H_ @}*/
