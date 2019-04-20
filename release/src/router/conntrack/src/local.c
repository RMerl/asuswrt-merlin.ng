/*
 * (C) 2006-2007 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Description: UNIX sockets library
 */

#include "local.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/un.h>

#define UNIX_SOCKET_BACKLOG 100

int local_server_create(struct local_server *server, struct local_conf *conf)
{
	int fd;
	socklen_t len;
	struct sockaddr_un local;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		return -1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &conf->reuseaddr, 
				sizeof(conf->reuseaddr)) == -1) {
		close(fd);
		unlink(conf->path);
		return -1;
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, conf->path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	unlink(conf->path);

	if (bind(fd, (struct sockaddr *) &local, len) == -1) {
		close(fd);
		unlink(conf->path);
		return -1;
	}

	if (listen(fd, UNIX_SOCKET_BACKLOG) == -1) {
		close(fd);
		unlink(conf->path);
		return -1;
	}

	server->fd = fd;
	strcpy(server->path, conf->path);

	return 0;
}

void local_server_destroy(struct local_server *server)
{
	unlink(server->path);
	close(server->fd);
}

int do_local_server_step(struct local_server *server, void *data, 
			 int (*process)(int fd, void *data))
{
	int rfd;
	struct sockaddr_un local;
	socklen_t sin_size = sizeof(struct sockaddr_un);

	rfd = accept(server->fd, (struct sockaddr *) &local, &sin_size);
	if (rfd == -1)
		return -1;

	/* This descriptor will be closed later, we ignore OK and errors */
	if (process(rfd, data) != LOCAL_RET_STOLEN)
		close(rfd);

	return 0;
}

int local_client_create(struct local_conf *conf)
{
	socklen_t len;
	struct sockaddr_un local;
	int fd;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		return -1;

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, conf->path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);

	if (connect(fd, (struct sockaddr *) &local, len) == -1) {
		close(fd);
		return -1;
	}

	return fd;
}

void local_client_destroy(int fd)
{
	close(fd);
}

int do_local_client_step(int fd, void (*process)(char *buf))
{
	char buf[1024];

	memset(buf, 0, sizeof(buf));
	while (recv(fd, buf, sizeof(buf)-1, 0) > 0) {
		buf[sizeof(buf)-1] = '\0';
		if (process)
			process(buf);
		memset(buf, 0, sizeof(buf));
	}

	return 0;
}

void local_step(char *buf)
{
	printf("%s", buf);
}

int do_local_request(int request,
		     struct local_conf *conf,
		     void (*step)(char *buf))
{
	int fd, ret;

	fd = local_client_create(conf);
	if (fd == -1)
		return -1;

	ret = send(fd, &request, sizeof(int), 0);
	if (ret == -1)
		goto err1;

	do_local_client_step(fd, step);

	local_client_destroy(fd);

	return 0;
err1:
	local_client_destroy(fd);
	return -1;
}
