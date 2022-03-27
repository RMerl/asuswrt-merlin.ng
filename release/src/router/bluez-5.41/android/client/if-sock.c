/*
 * Copyright (C) 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include "if-main.h"
#include "pollhandler.h"
#include "../hal-utils.h"

const btsock_interface_t *if_sock = NULL;

SINTMAP(btsock_type_t, -1, "(unknown)")
	DELEMENT(BTSOCK_RFCOMM),
	DELEMENT(BTSOCK_SCO),
	DELEMENT(BTSOCK_L2CAP),
ENDMAP

#define MAX_LISTEN_FD 15
static int listen_fd[MAX_LISTEN_FD];
static int listen_fd_count;

static const char * const uuids[] = {
	"00001101", "00001105", "0000112f", NULL
};

/*
 * This function reads data from file descriptor and
 * prints it to the user
 */
static void receive_from_client(struct pollfd *pollfd)
{
	char buf[16];
	/*
	 * Buffer for lines:
	 * 41 42 43 20 20 00 31 32 00 07 04 00 00 00 00 00 ABC  .12.....
	 */
	char outbuf[sizeof(buf) * 4 + 2];
	int i;
	int ret;

	if (pollfd->revents & POLLHUP) {
		haltest_error("Disconnected fd=%d\n", pollfd->fd);
		poll_unregister_fd(pollfd->fd, receive_from_client);
	} else if (pollfd->revents & POLLIN) {
		haltest_info("receiving from client fd=%d\n", pollfd->fd);

		do {
			memset(outbuf, ' ', sizeof(outbuf));
			outbuf[sizeof(outbuf) - 1] = 0;
			ret = recv(pollfd->fd, buf, sizeof(buf), MSG_DONTWAIT);

			for (i = 0; i < ret; ++i)
				sprintf(outbuf + i * 3, "%02X ",
							(unsigned) buf[i]);
			outbuf[i * 3] = ' ';
			for (i = 0; i < ret; ++i)
				sprintf(outbuf + 48 + i, "%c",
					(isprint(buf[i]) ? buf[i] : '.'));
			if (ret > 0)
				haltest_info("%s\n", outbuf);
		} while (ret > 0);
	} else {
		/* For now disconnect on all other events */
		haltest_error("Poll event %x\n", pollfd->revents);
		poll_unregister_fd(pollfd->fd, receive_from_client);
	}
}

/*
 * This function read from fd socket information about
 * connected socket
 */
static void receive_sock_connect_signal(struct pollfd *pollfd)
{
	sock_connect_signal_t cs;
	char addr_str[MAX_ADDR_STR_LEN];

	if (pollfd->revents & POLLIN) {
		int ret;

		poll_unregister_fd(pollfd->fd, receive_sock_connect_signal);
		ret = read(pollfd->fd, &cs, sizeof(cs));
		if (ret != sizeof(cs)) {
			haltest_info("Read on connect return %d\n", ret);
			return;
		}

		haltest_info("Connection to %s channel %d status=%d\n",
				bt_bdaddr_t2str(&cs.bd_addr, addr_str),
				cs.channel, cs.status);

		if (cs.status == 0)
			poll_register_fd(pollfd->fd, POLLIN,
							receive_from_client);
	}

	if (pollfd->revents & POLLHUP) {
		haltest_error("Disconnected fd=%d revents=0x%X\n", pollfd->fd,
				pollfd->revents);
		poll_unregister_fd(pollfd->fd, receive_sock_connect_signal);
	}
}

/*
 * This function read from fd socket information about
 * incoming connection and starts monitoring new connection
 * on file descriptor read from fd.
 */
static void read_accepted(int fd)
{
	int ret;
	struct msghdr msg;
	struct iovec iv;
	char cmsgbuf[CMSG_SPACE(1)];
	struct cmsghdr *cmsgptr;
	sock_connect_signal_t cs;
	int accepted_fd = -1;
	char addr_str[MAX_ADDR_STR_LEN];

	memset(&msg, 0, sizeof(msg));
	memset(&iv, 0, sizeof(iv));
	memset(cmsgbuf, 0, sizeof(cmsgbuf));

	iv.iov_base = &cs;
	iv.iov_len = sizeof(cs);

	msg.msg_iov = &iv;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);

	do {
		ret = recvmsg(fd, &msg, MSG_NOSIGNAL);
	} while (ret < 0 && errno == EINTR);

	if (ret < 16 ||
		(msg.msg_flags & (MSG_CTRUNC | MSG_OOB | MSG_ERRQUEUE)) != 0)
		haltest_error("Failed to accept connection\n");

	for (cmsgptr = CMSG_FIRSTHDR(&msg);
		cmsgptr != NULL; cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) {
		int count;

		if (cmsgptr->cmsg_level != SOL_SOCKET ||
			cmsgptr->cmsg_type != SCM_RIGHTS)
			continue;

		memcpy(&accepted_fd, CMSG_DATA(cmsgptr), sizeof(accepted_fd));
		count = ((cmsgptr->cmsg_len - CMSG_LEN(0)) / sizeof(int));

		if (count != 1)
			haltest_error("Failed to accept descriptors count=%d\n",
									count);

		break;
	}

	haltest_info("Incoming connection from %s channel %d status=%d fd=%d\n",
					bt_bdaddr_t2str(&cs.bd_addr, addr_str),
					cs.channel, cs.status, accepted_fd);
	poll_register_fd(accepted_fd, POLLIN, receive_from_client);
}

/* handles incoming connections on socket */
static void client_connected(struct pollfd *pollfd)
{
	haltest_info("client connected %x\n", pollfd->revents);

	if (pollfd->revents & POLLHUP)
		poll_unregister_fd(pollfd->fd, client_connected);
	else if (pollfd->revents & POLLIN)
		read_accepted(pollfd->fd);
}

/* listen */

static void listen_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(btsock_type_t);
		*enum_func = enum_defines;
	} else if (argc == 5) {
		*user = (void *) uuids;
		*enum_func = enum_strings;
	}
}

static void listen_p(int argc, const char **argv)
{
	btsock_type_t type;
	const char *service_name;
	bt_uuid_t service_uuid;
	int channel;
	int sock_fd = -1;
	int flags;

	RETURN_IF_NULL(if_sock);

	/* Socket type */
	if (argc < 3) {
		haltest_error("No socket type specified\n");
		return;
	}
	type = str2btsock_type_t(argv[2]);
	if ((int) type == -1)
		type = atoi(argv[2]);

	/* service name */
	if (argc < 4) {
		haltest_error("No service name specified\n");
		return;
	}
	service_name = argv[3];

	/* uuid */
	if (argc < 5) {
		haltest_error("No uuid specified\n");
		return;
	}
	str2bt_uuid_t(argv[4], &service_uuid);

	/* channel */
	channel = argc > 5 ? atoi(argv[5]) : 0;

	/* flags */
	flags = argc > 6 ? atoi(argv[6]) : 0;

	if (listen_fd_count >= MAX_LISTEN_FD) {
		haltest_error("Max (%d) listening sockets exceeded\n",
							listen_fd_count);
		return;
	}
	EXEC(if_sock->listen, type, service_name,
				&service_uuid.uu[0], channel, &sock_fd, flags);
	if (sock_fd > 0) {
		int channel = 0;
		int ret = read(sock_fd, &channel, 4);
		if (ret != 4)
			haltest_info("Read channel failed\n");
		haltest_info("Channel returned from first read %d\n", channel);
		listen_fd[listen_fd_count++] = sock_fd;
		poll_register_fd(sock_fd, POLLIN, client_connected);
	}
}

/* connect */

static void connect_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*enum_func = enum_devices;
	} else if (argc == 4) {
		*user = TYPE_ENUM(btsock_type_t);
		*enum_func = enum_defines;
	} else if (argc == 5) {
		*user = (void *) uuids;
		*enum_func = enum_strings;
	}
}

static void connect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;
	btsock_type_t type;
	bt_uuid_t uuid;
	int channel;
	int sock_fd = -1;
	int flags;

	/* Address */
	if (argc <= 2) {
		haltest_error("No address specified\n");
		return;
	}
	str2bt_bdaddr_t(argv[2], &addr);

	/* Socket type */
	if (argc <= 3) {
		haltest_error("No socket type specified\n");
		return;
	}
	type = str2btsock_type_t(argv[3]);
	if ((int) type == -1)
		type = atoi(argv[3]);

	/* uuid */
	if (argc <= 4) {
		haltest_error("No uuid specified\n");
		return;
	}
	str2bt_uuid_t(argv[4], &uuid);

	/* channel */
	if (argc <= 5) {
		haltest_error("No channel specified\n");
		return;
	}
	channel = atoi(argv[5]);

	/* flags */
	flags = argc <= 6 ? 0 : atoi(argv[6]);

	RETURN_IF_NULL(if_sock);

	EXEC(if_sock->connect, &addr, type, &uuid.uu[0], channel, &sock_fd,
									flags);
	if (sock_fd > 0) {
		int channel = 0;
		int ret = read(sock_fd, &channel, 4);

		if (ret != 4)
			haltest_info("Read channel failed\n");
		haltest_info("Channel returned from first read %d\n", channel);
		listen_fd[listen_fd_count++] = sock_fd;
		poll_register_fd(sock_fd, POLLIN, receive_sock_connect_signal);
	}
}

/* Methods available in btsock_interface_t */
static struct method methods[] = {
	STD_METHODCH(listen,
			"<sock_type> <srvc_name> <uuid> [<channel>] [<flags>]"),
	STD_METHODCH(connect,
			"<addr> <sock_type> <uuid> <channel> [<flags>]"),
	END_METHOD
};

const struct interface sock_if = {
	.name = "socket",
	.methods = methods
};
