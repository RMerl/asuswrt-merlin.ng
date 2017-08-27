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
 * @defgroup stream_unix stream_unix
 * @{ @ingroup streams
 */

#ifndef STREAM_UNIX_H_
#define STREAM_UNIX_H_

#include <sys/un.h>

/**
 * Create a stream for UNIX sockets.
 *
 * UNIX URIs start with unix://, followed by the socket path. For absolute
 * paths, an URI looks something like:
 *
 *   unix:///path/to/socket
 *
 * @param uri		UNIX socket specific URI, must start with "unix://"
 * @return			stream instance, NULL on failure
 */
stream_t *stream_create_unix(char *uri);

/**
 * Helper function to parse a unix:// URI to a sockaddr
 *
 * @param uri		URI
 * @param addr		sockaddr
 * @return			length of sockaddr, -1 on error
 */
int stream_parse_uri_unix(char *uri, struct sockaddr_un *addr);

#endif /** STREAM_UNIX_H_ @}*/
