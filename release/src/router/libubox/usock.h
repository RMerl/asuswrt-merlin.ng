/*
 * usock - socket helper functions
 *
 * Copyright (C) 2010 Steven Barth <steven@midlink.org>
 * Copyright (C) 2011-2012 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef USOCK_H_
#define USOCK_H_

#define USOCK_TCP 0
#define USOCK_UDP 1

#define USOCK_SERVER		0x0100
#define USOCK_NOCLOEXEC		0x0200
#define USOCK_NONBLOCK		0x0400
#define USOCK_NUMERIC		0x0800
#define USOCK_IPV6ONLY		0x2000
#define USOCK_IPV4ONLY		0x4000
#define USOCK_UNIX		0x8000

const char *usock_port(int port);
int usock(int type, const char *host, const char *service);
int usock_inet_timeout(int type, const char *host, const char *service,
		       void *addr, int timeout);
static inline int
usock_inet(int type, const char *host, const char *service, void *addr)
{
    return usock_inet_timeout(type, host, service, addr, -1);
}

/**
 * Wait for a socket to become ready.
 *
 * This may be useful for users of USOCK_NONBLOCK to wait (with a timeout)
 * for a socket.
 *
 * @param fd file descriptor of socket
 * @param msecs timeout in microseconds
 */
int usock_wait_ready(int fd, int msecs);

#endif /* USOCK_H_ */
