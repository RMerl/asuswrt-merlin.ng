/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/mainloop.h"
#include "btdev.h"
#include "server.h"

#define uninitialized_var(x) x = x

struct server {
	enum server_type type;
	uint16_t id;
	int fd;
};

struct client {
	int fd;
	struct btdev *btdev;
	uint8_t *pkt_data;
	uint8_t pkt_type;
	uint16_t pkt_expect;
	uint16_t pkt_len;
	uint16_t pkt_offset;
};

static void server_destroy(void *user_data)
{
	struct server *server = user_data;

	close(server->fd);

	free(server);
}

static void client_destroy(void *user_data)
{
	struct client *client = user_data;

	btdev_destroy(client->btdev);

	close(client->fd);

	free(client);
}

static void client_write_callback(const struct iovec *iov, int iovlen,
							void *user_data)
{
	struct client *client = user_data;
	struct msghdr msg;
	ssize_t written;

	memset(&msg, 0, sizeof(msg));

	msg.msg_iov = (struct iovec *) iov;
	msg.msg_iovlen = iovlen;

	written = sendmsg(client->fd, &msg, MSG_DONTWAIT);
	if (written < 0)
		return;
}

static void client_read_callback(int fd, uint32_t events, void *user_data)
{
	struct client *client = user_data;
	static uint8_t buf[4096];
	uint8_t *ptr = buf;
	ssize_t len;
	uint16_t count;

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(client->fd);
		return;
	}

again:
	len = recv(fd, buf + client->pkt_offset,
			sizeof(buf) - client->pkt_offset, MSG_DONTWAIT);
	if (len < 0) {
		if (errno == EAGAIN)
			goto again;
		return;
	}

	if (!client->btdev)
		return;

	count = client->pkt_offset + len;

	while (count > 0) {
		hci_command_hdr *cmd_hdr;
		hci_acl_hdr *acl_hdr;

		if (!client->pkt_data) {
			client->pkt_type = ptr[0];

			switch (client->pkt_type) {
			case HCI_COMMAND_PKT:
				if (count < HCI_COMMAND_HDR_SIZE + 1) {
					client->pkt_offset += len;
					return;
				}
				cmd_hdr = (hci_command_hdr *) (ptr + 1);
				client->pkt_expect = HCI_COMMAND_HDR_SIZE +
							cmd_hdr->plen + 1;
				client->pkt_data = malloc(client->pkt_expect);
				client->pkt_len = 0;
				break;
			case HCI_ACLDATA_PKT:
				acl_hdr = (hci_acl_hdr*)(ptr + 1);
				client->pkt_expect = HCI_ACL_HDR_SIZE + acl_hdr->dlen + 1;
				client->pkt_data = malloc(client->pkt_expect);
				client->pkt_len = 0;
				break;
			default:
				printf("packet error\n");
				return;
			}

			client->pkt_offset = 0;
		}

		if (count >= client->pkt_expect) {
			memcpy(client->pkt_data + client->pkt_len,
						ptr, client->pkt_expect);
			ptr += client->pkt_expect;
			count -= client->pkt_expect;

			btdev_receive_h4(client->btdev, client->pkt_data,
					client->pkt_len + client->pkt_expect);

			free(client->pkt_data);
			client->pkt_data = NULL;
		} else {
			memcpy(client->pkt_data + client->pkt_len, ptr, count);
			client->pkt_len += count;
			client->pkt_expect -= count;
			count = 0;
		}
	}
}

static int accept_client(int fd)
{
	struct sockaddr_un addr;
	socklen_t len;
	int nfd;

	memset(&addr, 0, sizeof(addr));
	len = sizeof(addr);

	if (getsockname(fd, (struct sockaddr *) &addr, &len) < 0) {
		perror("Failed to get socket name");
		return -1;
	}

	printf("Request for %s\n", addr.sun_path);

	nfd = accept(fd, (struct sockaddr *) &addr, &len);
	if (nfd < 0) {
		perror("Failed to accept client socket");
		return -1;
	}

	return nfd;
}

static void server_accept_callback(int fd, uint32_t events, void *user_data)
{
	struct server *server = user_data;
	struct client *client;
	enum btdev_type uninitialized_var(type);

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(server->fd);
		return;
	}

	client = malloc(sizeof(*client));
	if (!client)
		return;

	memset(client, 0, sizeof(*client));

	client->fd = accept_client(server->fd);
	if (client->fd < 0) {
		free(client);
		return;
	}

	switch (server->type) {
	case SERVER_TYPE_BREDRLE:
		type = BTDEV_TYPE_BREDRLE;
		break;
	case SERVER_TYPE_BREDR:
		type = BTDEV_TYPE_BREDR;
		break;
	case SERVER_TYPE_LE:
		type = BTDEV_TYPE_LE;
		break;
	case SERVER_TYPE_AMP:
		type = BTDEV_TYPE_AMP;
		break;
	case SERVER_TYPE_MONITOR:
		goto done;
	}

	client->btdev = btdev_create(type, server->id);
	if (!client->btdev) {
		close(client->fd);
		free(client);
		return;
	}

	btdev_set_send_handler(client->btdev, client_write_callback, client);

done:
	if (mainloop_add_fd(client->fd, EPOLLIN, client_read_callback,
						client, client_destroy) < 0) {
		btdev_destroy(client->btdev);
		close(client->fd);
		free(client);
	}
}

static int open_unix(const char *path)
{
	struct sockaddr_un addr;
	int fd;

	unlink(path);

	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("Failed to open server socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path);

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind server socket");
		close(fd);
		return -1;
	}

	if (listen(fd, 5) < 0) {
		perror("Failed to listen server socket");
		close(fd);
		return -1;
	}

	return fd;
}

struct server *server_open_unix(enum server_type type, const char *path)
{
	struct server *server;

	server = malloc(sizeof(*server));
	if (!server)
		return NULL;

	memset(server, 0, sizeof(*server));
	server->type = type;
	server->id = 0x42;

	server->fd = open_unix(path);
	if (server->fd < 0) {
		free(server);
		return NULL;
	}

	if (mainloop_add_fd(server->fd, EPOLLIN, server_accept_callback,
						server, server_destroy) < 0) {
		close(server->fd);
		free(server);
		return NULL;
	}

	return server;
}

static int open_tcp(void)
{
	struct sockaddr_in addr;
	int fd, opt = 1;

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("Failed to open server socket");
		return -1;
	}

	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(45550);

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind server socket");
		close(fd);
		return -1;
	}

	if (listen(fd, 5) < 0) {
		perror("Failed to listen server socket");
		close(fd);
		return -1;
	}

	return fd;
}

struct server *server_open_tcp(enum server_type type)
{
	struct server *server;

	server = malloc(sizeof(*server));
	if (!server)
		return server;

	memset(server, 0, sizeof(*server));
	server->type = type;
	server->id = 0x43;

	server->fd = open_tcp();
	if (server->fd < 0) {
		free(server);
		return NULL;
	}

	if (mainloop_add_fd(server->fd, EPOLLIN, server_accept_callback,
						server, server_destroy) < 0) {
		close(server->fd);
		free(server);
		return NULL;
	}

	return server;
}

void server_close(struct server *server)
{
	if (!server)
		return;

	mainloop_remove_fd(server->fd);
}
