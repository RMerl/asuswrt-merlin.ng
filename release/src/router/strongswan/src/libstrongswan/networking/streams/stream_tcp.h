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
 * @defgroup stream_tcp stream_tcp
 * @{ @ingroup streams
 */

#ifndef STREAM_TCP_H_
#define STREAM_TCP_H_

#include <library.h>

/**
 * Create a stream for TCP sockets.
 *
 * TCP URIs start with tcp://, followed by a hostname (FQDN or IP), followed
 * by a colon separated port. A full TCP uri looks something like:
 *
 *   tcp://srv.example.com:5555
 *   tcp://0.0.0.0:1234
 *   tcp://[fec2::1]:7654
 *
 * There is no default port, so a colon after tcp:// is mandatory.
 *
 * @param uri		TCP socket specific URI, must start with "tcp://"
 * @return			stream instance, NULL on failure
 */
stream_t *stream_create_tcp(char *uri);

/**
 * Helper function to parse a tcp:// URI to a sockaddr
 *
 * @param uri		URI
 * @param addr		sockaddr, large enough for URI
 * @return			length of sockaddr, -1 on error
 */
int stream_parse_uri_tcp(char *uri, struct sockaddr *addr);

#endif /** STREAM_TCP_H_ @}*/
