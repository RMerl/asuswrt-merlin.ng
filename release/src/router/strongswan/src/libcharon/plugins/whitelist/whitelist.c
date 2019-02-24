/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "whitelist_msg.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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
		strcpy(addr.un.sun_path, WHITELIST_SOCKET);

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

static int read_all(int fd, void *buf, size_t len)
{
	ssize_t ret, done = 0;

	while (done < len)
	{
		ret = read(fd, buf, len - done);
		if (ret == -1 && errno == EINTR)
		{	/* interrupted, try again */
			continue;
		}
		if (ret < 0)
		{
			return -1;
		}
		done += ret;
		buf += ret;
	}
	return len;
}

static int write_all(int fd, void *buf, size_t len)
{
	ssize_t ret, done = 0;

	while (done < len)
	{
		ret = write(fd, buf, len - done);
		if (ret == -1 && errno == EINTR)
		{	/* interrupted, try again */
			continue;
		}
		if (ret < 0)
		{
			return -1;
		}
		done += ret;
		buf += ret;
	}
	return len;
}

/**
 * Send a single message
 */
static int send_msg(int type, char *id)
{
	whitelist_msg_t msg = {
		.type = htonl(type),
	};
	int fd;

	fd = make_connection();
	if (fd == -1)
	{
		return 2;
	}
	snprintf(msg.id, sizeof(msg.id), "%s", id);
	if (write_all(fd, &msg, sizeof(msg)) != sizeof(msg))
	{
		fprintf(stderr, "writing to socket failed: %s\n", strerror(errno));
		close(fd);
		return 2;
	}
	if (type == WHITELIST_LIST)
	{
		while (1)
		{
			if (read_all(fd, &msg, sizeof(msg)) != sizeof(msg))
			{
				fprintf(stderr, "reading failed: %s\n", strerror(errno));
				close(fd);
				return 2;
			}
			if (ntohl(msg.type) != WHITELIST_LIST)
			{
				break;
			}
			msg.id[sizeof(msg.id) - 1] = '\0';
			printf("%s\n", msg.id);
		}
	}
	close(fd);
	return 0;
}

/**
 * Send a batch of messages, reading identities from a file
 */
static int send_batch(int type, char *file)
{
	whitelist_msg_t msg = {
		.type = htonl(type),
	};
	FILE *f = stdin;
	int fd, len;

	fd = make_connection();
	if (fd == -1)
	{
		return 2;
	}
	if (file)
	{
		f = fopen(file, "r");
		if (f == NULL)
		{
			fprintf(stderr, "opening %s failed: %s\n", file, strerror(errno));
			close(fd);
			return 3;
		}
	}
	while (fgets(msg.id, sizeof(msg.id), f))
	{
		len = strlen(msg.id);
		if (len == 0)
		{
			continue;
		}
		if (msg.id[len-1] == '\n')
		{
			msg.id[len-1] = '\0';
		}
		if (write_all(fd, &msg, sizeof(msg)) != sizeof(msg))
		{
			fprintf(stderr, "writing to socket failed: %s\n", strerror(errno));
			if (f != stdin)
			{
				fclose(f);
			}
			close(fd);
			return 2;
		}
	}
	if (f != stdin)
	{
		fclose(f);
	}
	close(fd);
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc == 3 && strcmp(argv[1], "add") == 0)
	{
		return send_msg(WHITELIST_ADD, argv[2]);
	}
	if (argc == 3 && strcmp(argv[1], "remove") == 0)
	{
		return send_msg(WHITELIST_REMOVE, argv[2]);
	}
	if ((argc == 2 || argc == 3) && strcmp(argv[1], "add-from") == 0)
	{
		return send_batch(WHITELIST_ADD, argc == 3 ? argv[2] : NULL);
	}
	if ((argc == 2 || argc == 3) && strcmp(argv[1], "remove-from") == 0)
	{
		return send_batch(WHITELIST_REMOVE, argc == 3 ? argv[2] : NULL);
	}
	if ((argc == 2 || argc == 3) && strcmp(argv[1], "flush") == 0)
	{
		return send_msg(WHITELIST_FLUSH, argc == 3 ? argv[2] : "%any");
	}
	if ((argc == 2 || argc == 3) && strcmp(argv[1], "list") == 0)
	{
		return send_msg(WHITELIST_LIST, argc == 3 ? argv[2] : "%any");
	}
	if (argc == 2 && strcmp(argv[1], "enable") == 0)
	{
		return send_msg(WHITELIST_ENABLE, "");
	}
	if (argc == 2 && strcmp(argv[1], "disable") == 0)
	{
		return send_msg(WHITELIST_DISABLE, "");
	}
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  %s add <identity>\n", argv[0]);
	fprintf(stderr, "  %s remove <identity>\n", argv[0]);
	fprintf(stderr, "  %s add-from <file>\n", argv[0]);
	fprintf(stderr, "  %s remove-from <file>\n", argv[0]);
	fprintf(stderr, "  %s flush [<pattern>]\n", argv[0]);
	fprintf(stderr, "  %s list [<pattern>]\n", argv[0]);
	fprintf(stderr, "  %s enable\n", argv[0]);
	fprintf(stderr, "  %s disable\n", argv[0]);
	return 1;
}
