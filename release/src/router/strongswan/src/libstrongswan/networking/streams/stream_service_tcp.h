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
 * @defgroup stream_service_tcp stream_service_tcp
 * @{ @ingroup stream
 */

#ifndef STREAM_SERVICE_TCP_H_
#define STREAM_SERVICE_TCP_H_

/**
 * Create a service instance for TCP sockets.
 *
 * @param uri		TCP socket specific URI, must start with "tcp://"
 * @param backlog	size of the backlog queue, as passed to listen()
 * @return			stream_service instance, NULL on failure
 */
stream_service_t *stream_service_create_tcp(char *uri, int backlog);

#endif /** STREAM_SERVICE_TCP_H_ @}*/
