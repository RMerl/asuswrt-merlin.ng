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
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include "stream_tcp.h"

/**
 * See header.
 */
int stream_parse_uri_tcp(char *uri, struct sockaddr *addr)
{
	char *pos, buf[128];
	host_t *host;
	u_long port;
	int len;

	if (!strpfx(uri, "tcp://"))
	{
		return -1;
	}
	uri += strlen("tcp://");
	pos = strrchr(uri, ':');
	if (!pos)
	{
		return -1;
	}
	if (*uri == '[' && pos > uri && *(pos - 1) == ']')
	{
		/* IPv6 URI */
		snprintf(buf, sizeof(buf), "%.*s", (int)(pos - uri - 2), uri + 1);
	}
	else
	{
		snprintf(buf, sizeof(buf), "%.*s", (int)(pos - uri), uri);
	}
	port = strtoul(pos + 1, &pos, 10);
	if (port == ULONG_MAX || *pos || port > 65535)
	{
		return -1;
	}
	host = host_create_from_dns(buf, AF_UNSPEC, port);
	if (!host)
	{
		return -1;
	}
	len = *host->get_sockaddr_len(host);
	memcpy(addr, host->get_sockaddr(host), len);
	host->destroy(host);
	return len;
}

/**
 * See header
 */
stream_t *stream_create_tcp(char *uri)
{
	union {
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
		struct sockaddr sa;
	} addr;
	int fd, len;

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
	if (connect(fd, &addr.sa, len))
	{
		DBG1(DBG_NET, "connecting to '%s' failed: %s", uri, strerror(errno));
		close(fd);
		return NULL;
	}
	return stream_create_from_fd(fd);
}
