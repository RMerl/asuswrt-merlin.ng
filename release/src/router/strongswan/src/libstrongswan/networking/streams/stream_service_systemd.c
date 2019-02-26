/*
 * Copyright (C) 2017 aszlig
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <systemd/sd-daemon.h>

#include <library.h>

/**
 * See header
 */
stream_service_t *stream_service_create_systemd(char *uri, int backlog)
{
#ifndef HAVE_SD_LISTEN_FDS_WITH_NAMES
	DBG1(DBG_NET, "unable to open stream URI '%s': named systemd sockets not "
		 "supported", uri);
	return NULL;
#else
	int i, num_fds, fd;
	char **fdmap;

	if (!strpfx(uri, "systemd://"))
	{
		DBG1(DBG_NET, "invalid stream URI: '%s'", uri);
		return NULL;
	}
	uri += strlen("systemd://");

	num_fds = sd_listen_fds_with_names(0, &fdmap);
	if (num_fds <= 0)
	{
		DBG1(DBG_NET, "no systemd sockets for '%s'", uri);
		return NULL;
	}

	for (i = 0, fd = -1; i < num_fds; i++)
	{
		if (fd == -1 && streq(fdmap[i], uri))
		{
			fd = SD_LISTEN_FDS_START + i;
		}
		free(fdmap[i]);
	}
	free(fdmap);

	if (fd == -1)
	{
		DBG1(DBG_NET, "unable to find systemd FD for '%s'", uri);
		return NULL;
	}
	return stream_service_create_from_fd(fd);
#endif
}
