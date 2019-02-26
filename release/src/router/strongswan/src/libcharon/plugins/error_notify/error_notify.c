/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "error_notify_msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <arpa/inet.h>

/**
 * Connect to the daemon, return FD
 */
static int make_connection()
{
	union {
		struct sockaddr_un un;
		struct sockaddr_in in;
		struct sockaddr sa;
	} addr;
	int fd, len;

	if (getenv("TCP_PORT"))
	{
		addr.in.sin_family = AF_INET;
		addr.in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		addr.in.sin_port = htons(atoi(getenv("TCP_PORT")));
		len = sizeof(addr.in);
	}
	else
	{
		addr.un.sun_family = AF_UNIX;
		strcpy(addr.un.sun_path, ERROR_NOTIFY_SOCKET);

		len = offsetof(struct sockaddr_un, sun_path) + strlen(addr.un.sun_path);
	}
	fd = socket(addr.sa.sa_family, SOCK_STREAM, 0);
	if (fd < 0)
	{
		fprintf(stderr, "opening socket failed: %s\n", strerror(errno));
		return -1;
	}
	if (connect(fd, &addr.sa, len) < 0)
	{
		fprintf(stderr, "connecting failed: %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	return fd;
}

/**
 * Example of a simple notification listener
 */
int main(int argc, char *argv[])
{
	error_notify_msg_t msg;
	int s, len, total;
	void *pos;

	s = make_connection();
	if (s < 0)
	{
		return 1;
	}
	while (1)
	{
		total = 0;
		pos = &msg;

		while (total < sizeof(msg))
		{
			len = read(s, pos, sizeof(msg) - total);
			if (len < 0)
			{
				fprintf(stderr, "read failed: %s\n", strerror(errno));
				close(s);
				return 1;
			}
			total += len;
			pos += len;
		}
		printf("%d %s %s %s %s\n",
			   ntohl(msg.type), msg.name, msg.id, msg.ip, msg.str);
	}
	close(s);
	return 0;
}
