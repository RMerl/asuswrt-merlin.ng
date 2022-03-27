/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "systemd.h"

int sd_listen_fds(int unset_environment)
{
	return 0;
}

int sd_notify(int unset_environment, const char *state)
{
	const char *sock;
	struct sockaddr_un addr;
	struct msghdr msghdr;
	struct iovec iovec;
	int fd, err;

	if (!state) {
		err = -EINVAL;
		goto done;
	}

	sock = getenv("NOTIFY_SOCKET");
	if (!sock)
		return 0;

	/* check for abstract socket or absolute path */
	if (sock[0] != '@' && sock[0] != '/') {
		err = -EINVAL;
		goto done;
	}

	fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		err = -errno;
		goto done;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock, sizeof(addr.sun_path));

	if (addr.sun_path[0] == '@')
		addr.sun_path[0] = '\0';

	memset(&iovec, 0, sizeof(iovec));
	iovec.iov_base = (char *) state;
	iovec.iov_len = strlen(state);

	memset(&msghdr, 0, sizeof(msghdr));
	msghdr.msg_name = &addr;
	msghdr.msg_namelen = offsetof(struct sockaddr_un, sun_path) +
								strlen(sock);

	if (msghdr.msg_namelen > sizeof(struct sockaddr_un))
		msghdr.msg_namelen = sizeof(struct sockaddr_un);

	msghdr.msg_iov = &iovec;
	msghdr.msg_iovlen = 1;

	if (sendmsg(fd, &msghdr, MSG_NOSIGNAL) < 0)
		err = -errno;
	else
		err = 1;

	close(fd);

done:
	if (unset_environment)
		unsetenv("NOTIFY_SOCKET");

	return err;
}
