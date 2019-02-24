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
 * @defgroup stream_service stream_service
 * @{ @ingroup streams
 */

#ifndef STREAM_SERVICE_H_
#define STREAM_SERVICE_H_

typedef struct stream_service_t stream_service_t;

#include <processing/jobs/job.h>
#include <networking/streams/stream.h>

/**
 * Constructor function prototype for stream_service_t.
 *
 * @param uri			URI to create a stream for
 * @param backlog		size of the backlog queue, as passed to listen()
 * @return				stream instance, NULL on error
 */
typedef stream_service_t*(*stream_service_constructor_t)(char *uri, int backlog);

/**
 * Service callback routine for accepting client connections.
 *
 * The passed stream gets closed/destroyed by the callback caller, unless
 * TRUE is returned.
 *
 * @param data			user data, as passed during registration
 * @param stream		accept()ed client connection
 * @return				TRUE to keep stream alive, FALSE to destroy it
 */
typedef bool (*stream_service_cb_t)(void *data, stream_t *stream);

/**
 * A service accepting client connection streams.
 */
struct stream_service_t {

	/**
	 * Start accepting client connections on this stream service.
	 *
	 * To stop accepting connections, pass a NULL callback function.
	 *
	 * @param cb		callback function to call for accepted client streams
	 * @param data		data to pass to callback function
	 * @param prio		job priority to run callback with
	 * @param cncrncy	maximum number of parallel callback invocations
	 */
	void (*on_accept)(stream_service_t *this,
					  stream_service_cb_t cb, void *data,
					  job_priority_t prio, u_int cncrncy);

	/**
	 * Destroy a stream_service_t.
	 */
	void (*destroy)(stream_service_t *this);
};

/**
 * Create a service from a file descriptor.
 *
 * The file descriptor MUST be a socket.
 *
 * @param fd		file descriptor to wrap into a stream_service_t
 * @return			stream_service instance
 */
stream_service_t *stream_service_create_from_fd(int fd);

#endif /** STREAM_SERVICE_H_ @}*/
