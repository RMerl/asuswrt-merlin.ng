/*
 * Copyright (C) 2024 Tobias Brunner
 * Copyright (C) 2024 Thomas Egerer
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

#include <errno.h>
#include <limits.h>
#include <sys/socket.h>
#include <linux/vm_sockets.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <library.h>

#include "stream_vsock.h"

/**
 * Helper function to parse a vsock:// URI to a sockaddr.
 * Returns the length of the sockaddr or -1.
 */
static int stream_parse_uri_vsock(char *uri, struct sockaddr_vm *addr)
{
	unsigned long cid, port;

	if (!strpfx(uri, "vsock://"))
	{
		return -1;
	}

	uri += strlen("vsock://");
	if (*uri == '*')
	{
		cid = VMADDR_CID_ANY;
		uri++;
	}
	else
	{
		cid = strtoul(uri, &uri, 10);
	}

	if (*uri != ':' || cid > UINT_MAX)
	{
		return -1;
	}

	port = strtoul(uri + 1, &uri, 10);
	if (port > UINT_MAX || *uri)
	{
		return -1;
	}

	*addr = (struct sockaddr_vm){
		.svm_family = AF_VSOCK,
		.svm_port = port,
		.svm_cid = cid,
	};
	return sizeof(*addr);
}

/*
 * Described in header
 */
int stream_initialize_socket_vsock(char *uri, int *backlog)
{
	int fd, len;
	struct sockaddr_vm addr;

	len = stream_parse_uri_vsock(uri, &addr);
	if (len == -1)
	{
		DBG1(DBG_NET, "invalid stream URI: '%s'", uri);
		return -1;
	}

	fd = socket(AF_VSOCK, SOCK_STREAM, 0);
	if (fd == -1)
	{
		DBG1(DBG_NET, "opening socket '%s' failed: %s", uri, strerror(errno));
		return -1;
	}

	if (backlog)
	{
		if (bind(fd, (struct sockaddr*)&addr, len) < 0)
		{
			DBG1(DBG_NET, "binding socket '%s' failed: %s", uri,
				 strerror(errno));
			close(fd);
			return -1;
		}

		if (listen(fd, *backlog) < 0)
		{
			DBG1(DBG_NET, "listen on socket '%s' failed: %s", uri,
				 strerror(errno));
			close(fd);
			return -1;
		}
	}
	else
	{
		if (connect(fd, (struct sockaddr*)&addr, len) < 0)
		{
			DBG1(DBG_NET, "connecting to '%s' failed: %s", uri,
				 strerror(errno));
			close(fd);
			return -1;
		}
	}
	return fd;
}

/*
 * Described in header
 */
stream_t *stream_create_vsock(char *uri)
{
	int fd = stream_initialize_socket_vsock(uri, NULL);

	return (fd == -1) ? NULL : stream_create_from_fd(fd);
}
