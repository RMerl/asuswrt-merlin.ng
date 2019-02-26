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
#include <networking/streams/stream_unix.h>

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

/**
 * See header
 */
stream_service_t *stream_service_create_unix(char *uri, int backlog)
{
	struct sockaddr_un addr;
	mode_t old;
	int fd, len;

	len = stream_parse_uri_unix(uri, &addr);
	if (len == -1)
	{
		DBG1(DBG_NET, "invalid stream URI: '%s'", uri);
		return NULL;
	}
	if (!lib->caps->check(lib->caps, CAP_CHOWN))
	{	/* required to chown(2) service socket */
		DBG1(DBG_NET, "cannot change ownership of socket '%s' without "
			 "CAP_CHOWN capability. socket directory should be accessible to "
			 "UID/GID under which the daemon will run", uri);
	}
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
	{
		DBG1(DBG_NET, "opening socket '%s' failed: %s", uri, strerror(errno));
		return NULL;
	}
	unlink(addr.sun_path);

	old = umask(S_IRWXO);
	if (bind(fd, (struct sockaddr*)&addr, len) < 0)
	{
		DBG1(DBG_NET, "binding socket '%s' failed: %s", uri, strerror(errno));
		close(fd);
		return NULL;
	}
	umask(old);
	/* Only attempt to change owner of socket if we have CAP_CHOWN. Otherwise,
	 * attempt to change group of socket to group under which charon runs after
	 * dropping caps. This requires the user that charon starts as to:
	 * a) Have write access to the socket dir.
	 * b) Belong to the group that charon will run under after dropping caps. */
	if (lib->caps->check(lib->caps, CAP_CHOWN))
	{
		if (chown(addr.sun_path, lib->caps->get_uid(lib->caps),
				  lib->caps->get_gid(lib->caps)) != 0)
		{
			DBG1(DBG_NET, "changing socket owner/group for '%s' failed: %s",
				 uri, strerror(errno));
		}
	}
	else
	{
		if (chown(addr.sun_path, -1, lib->caps->get_gid(lib->caps)) != 0)
		{
			DBG1(DBG_NET, "changing socket group for '%s' failed: %s",
				 uri, strerror(errno));
		}
	}
	if (listen(fd, backlog) < 0)
	{
		DBG1(DBG_NET, "listen on socket '%s' failed: %s", uri, strerror(errno));
		unlink(addr.sun_path);
		close(fd);
		return NULL;
	}
	return stream_service_create_from_fd(fd);
}
