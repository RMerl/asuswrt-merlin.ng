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

#include "stream_unix.h"

/**
 * See header
 */
int stream_parse_uri_unix(char *uri, struct sockaddr_un *addr)
{
	if (!strpfx(uri, "unix://"))
	{
		return -1;
	}
	uri += strlen("unix://");

	memset(addr, 0, sizeof(*addr));
	addr->sun_family = AF_UNIX;
	strncpy(addr->sun_path, uri, sizeof(addr->sun_path));
	addr->sun_path[sizeof(addr->sun_path)-1] = '\0';

	return offsetof(struct sockaddr_un, sun_path) + strlen(addr->sun_path);
}

/**
 * See header
 */
stream_t *stream_create_unix(char *uri)
{
	struct sockaddr_un addr;
	int len, fd;

	len = stream_parse_uri_unix(uri, &addr);
	if (len == -1)
	{
		DBG1(DBG_NET, "invalid stream URI: '%s'", uri);
		return NULL;
	}
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
	{
		DBG1(DBG_NET, "opening socket '%s' failed: %s", uri, strerror(errno));
		return NULL;
	}
	if (connect(fd, (struct sockaddr*)&addr, len) < 0)
	{
		DBG1(DBG_NET, "connecting to '%s' failed: %s", uri, strerror(errno));
		close(fd);
		return NULL;
	}
	return stream_create_from_fd(fd);
}
