/* $Id$ */

/*
 * fakepoll.c
 *
 * On systems where 'poll' doesn't exist, fake it with 'select'.
 *
 * Copyright (c) 2001-2003, Nick Mathewson <nickm@freehaven.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 *
 *   * Neither the names of the copyright owners nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defines.h"
#include "config.h"
#include "common.h"

/* prevents ISO C error */
#ifdef USE_FAKE_POLL
static void
FAKEPOLL(int stop)
{
    if (!stop)
        FAKEPOLL(1);
    return;
}

#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#if _MSC_VER > 1300
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(_MSC_VER)
#include <winsock.h>
#endif

/* by default, windows handles only 64 fd's */
#if defined(MS_WINDOWS) && !defined(FD_SETSIZE)
#define FD_SETSIZE MAXCONNECTIONS
#endif

#include "util.h"

/**
 * custom version of poll() using select() in the backend
 * only used if you don't actually have poll on your system.
 */
int
poll(struct pollfd *ufds, unsigned int nfds, int timeout)
{
    int idx, maxfd, fd;
    int r;
#ifdef MS_WINDOWS
    int any_fds_set = 0;
#endif
    fd_set readfds, writefds, exceptfds;
#ifdef USING_FAKE_TIMEVAL
#undef timeval
#undef tv_sec
#undef tv_usec
#endif
    struct timeval _timeout;
    _timeout.tv_sec = timeout / 1000;
    _timeout.tv_usec = (timeout % 1000) * 1000;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    maxfd = -1;
    for (idx = 0; idx < nfds; ++idx) {
        ufds[idx].revents = 0;
        fd = ufds[idx].fd;
        if (fd > maxfd) {
            maxfd = fd;
#ifdef MS_WINDOWS
            any_fds_set = 1;
#endif
        }
        if (ufds[idx].events & POLLIN)
            FD_SET(fd, &readfds);
        if (ufds[idx].events & POLLOUT)
            FD_SET(fd, &writefds);
        FD_SET(fd, &exceptfds);
    }
#ifdef MS_WINDOWS
    if (!any_fds_set) {
        Sleep(timeout);
        return 0;
    }
#endif
    r = select(maxfd + 1, &readfds, &writefds, &exceptfds, timeout == -1 ? NULL : &_timeout);
    if (r <= 0)
        return r;
    r = 0;
    for (idx = 0; idx < nfds; ++idx) {
        fd = ufds[idx].fd;
        if (FD_ISSET(fd, &readfds))
            ufds[idx].revents |= POLLIN;
        if (FD_ISSET(fd, &writefds))
            ufds[idx].revents |= POLLOUT;
        if (FD_ISSET(fd, &exceptfds))
            ufds[idx].revents |= POLLERR;
        if (ufds[idx].revents)
            ++r;
    }
    return r;
}
#endif
