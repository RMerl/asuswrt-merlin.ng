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

#include <library.h>
#include <networking/streams/stream_tcp.h>

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * See header
 */
stream_service_t *stream_service_create_tcp(char *uri, int backlog)
{
	union {
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
		struct sockaddr sa;
	} addr;
	int fd, len, on = 1;

	len = stream_parse_uri_tcp(uri, &addr.sa);
	if (len == -1)
	{
		DBG1(DBG_NET, "invalid stream URI: '%s'", uri);
		return NULL;
	}
	fd = socket(addr.sa.sa_family, SOCK_STREAM, 0);
	if (fd < 0)
	{
		DBG1(DBG_NET, "opening socket '%s' failed: %s", uri, strerror(errno));
		return NULL;
	}
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) != 0)
	{
		DBG1(DBG_NET, "SO_REUSADDR on '%s' failed: %s", uri, strerror(errno));
	}
	if (bind(fd, &addr.sa, len) < 0)
	{
		DBG1(DBG_NET, "binding socket '%s' failed: %s", uri, strerror(errno));
		close(fd);
		return NULL;
	}
	if (listen(fd, backlog) < 0)
	{
		DBG1(DBG_NET, "listen on socket '%s' failed: %s", uri, strerror(errno));
		close(fd);
		return NULL;
	}
	return stream_service_create_from_fd(fd);
}
