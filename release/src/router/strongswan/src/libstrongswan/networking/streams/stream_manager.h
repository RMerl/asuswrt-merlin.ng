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
 * @defgroup stream_manager stream_manager
 * @{ @ingroup streams
 */

#ifndef STREAM_MANAGER_H_
#define STREAM_MANAGER_H_

typedef struct stream_manager_t stream_manager_t;

#include <library.h>
#include <networking/streams/stream_service.h>

/**
 * Manages client-server connections and services using stream_t backends.
 */
struct stream_manager_t {

	/**
	 * Create a client-server connection to a service.
	 *
	 * @param uri		URI of service to connect to
	 * @return			stream instance, NULL on error
	 */
	stream_t* (*connect)(stream_manager_t *this, char *uri);

	/**
	 * Create a new service under an URI to accept() client connections.
	 *
	 * @param uri		URI of service to provide
	 * @param backlog	size of the backlog queue, as passed to listen()
	 * @return			service, NULL on error
	 */
	stream_service_t* (*create_service)(stream_manager_t *this, char *uri,
										int backlog);

	/**
	 * Register a stream backend to the manager.
	 *
	 * @param prefix	prefix of URIs to use the backend for
	 * @param create	constructor function for the stream
	 */
	void (*add_stream)(stream_manager_t *this, char *prefix,
					   stream_constructor_t create);

	/**
	 * Unregister stream backends from the manager.
	 *
	 * @param create	constructor function passed to add_stream()
	 */
	void (*remove_stream)(stream_manager_t *this, stream_constructor_t create);

	/**
	 * Register a stream service backend to the manager.
	 *
	 * @param prefix	prefix of URIs to use the backend for
	 * @param create	constructor function for the stream service
	 */
	void (*add_service)(stream_manager_t *this, char *prefix,
						stream_service_constructor_t create);

	/**
	 * Unregister stream service backends from the manager.
	 *
	 * @param create	constructor function passed to add_service()
	 */
	void (*remove_service)(stream_manager_t *this,
						   stream_service_constructor_t create);

	/**
	 * Destroy a stream_manager_t.
	 */
	void (*destroy)(stream_manager_t *this);
};

/**
 * Create a stream_manager instance.
 */
stream_manager_t *stream_manager_create();

#endif /** STREAM_MANAGER_H_ @}*/
