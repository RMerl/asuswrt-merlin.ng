/* Interface for TCP functions
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/nameser.h>
#include <net/if.h>
#include <netinet/in.h>
#include <resolv.h>

#include "log.h"
#include "tcp.h"

int tcp_construct(tcp_sock_t *tcp)
{
	ASSERT(tcp);

	memset(tcp, 0, sizeof(tcp_sock_t));

	tcp->initialized = 0;
	tcp->socket      = -1; /* Initialize to 'error', not a possible socket id. */
	tcp->timeout     = TCP_DEFAULT_TIMEOUT;

	return 0;
}

int tcp_destruct(tcp_sock_t *tcp)
{
	ASSERT(tcp);

	if (tcp->initialized == 1)
		tcp_exit(tcp);

	return 0;
}

static int soerror(int sd)
{
	int code = 0;
	socklen_t len = sizeof(code);

	if (getsockopt(sd, SOL_SOCKET, SO_ERROR, &code, &len))
		return 1;

	return errno = code;
}

/*
 * In the wonderful world of network programming the manual states that
 * EINPROGRESS is only a possible error on non-blocking sockets.  Real world
 * experience, however, suggests otherwise.  Simply poll() for completion and
 * then continue. --Joachim
 */
static int check_error(int sd, int msec)
{
	struct pollfd pfd = { sd, POLLOUT, 0 };

	if (EINPROGRESS == errno) {
		logit(LOG_INFO, "Waiting (%d sec) for three-way handshake to complete ...", msec / 1000);
		if (poll (&pfd, 1, msec) > 0 && !soerror(sd)) {
			logit(LOG_INFO, "Connected.");
			return 0;
		}
	}

	return 1;
}

static void set_timeouts(int sd, int timeout)
{
	struct timeval sv;

	memset(&sv, 0, sizeof(sv));
	sv.tv_sec  =  timeout / 1000;
	sv.tv_usec = (timeout % 1000) * 1000;
	if (-1 == setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &sv, sizeof(sv)))
		logit(LOG_INFO, "Failed setting receive timeout socket option: %s", strerror(errno));
	if (-1 == setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &sv, sizeof(sv)))
		logit(LOG_INFO, "Failed setting send timeout socket option: %s", strerror(errno));
}

int tcp_init(tcp_sock_t *tcp, char *msg)
{
	int rc = 0;

	ASSERT(tcp);

	if (tcp->initialized == 1)
		return 0;

	do {
		int s, sd, tries = 0;
		char port[10];
		char host[NI_MAXHOST];
		struct addrinfo hints, *servinfo, *ai;
		struct sockaddr *sa;
		socklen_t len;

		/* remote address */
		if (!tcp->remote_host)
			break;

		/* Clear DNS cache before calling getaddrinfo(). */
		res_init();

		/* Obtain address(es) matching host/port */
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;		/* Allow IPv4 or IPv6 */
		hints.ai_socktype = SOCK_STREAM;	/* Stream socket */
		hints.ai_flags = AI_NUMERICSERV;	/* No service name lookup */
		snprintf(port, sizeof(port), "%d", tcp->port);

		s = getaddrinfo(tcp->remote_host, port, &hints, &servinfo);
		if (s != 0 || !servinfo) {
			logit(LOG_WARNING, "Failed resolving hostname %s: %s", tcp->remote_host, gai_strerror(s));
			rc = RC_TCP_INVALID_REMOTE_ADDR;
			break;
		}
		ai = servinfo;

		while (1) {
			sd = socket(ai->ai_family, SOCK_STREAM, 0);
			if (sd == -1) {
				logit(LOG_ERR, "Error creating client socket: %s", strerror(errno));
				rc = RC_TCP_SOCKET_CREATE_ERROR;
				break;
			}

			/* Now we try connecting to the server, on connect fail, try next DNS record */
			sa  = ai->ai_addr;
			len = ai->ai_addrlen;

			if (getnameinfo(sa, len, host, sizeof(host), NULL, 0, NI_NUMERICHOST))
				goto next;

			set_timeouts(sd, tcp->timeout);

			logit(LOG_INFO, "%s, %sconnecting to %s([%s]:%d)", msg, tries ? "re" : "",
			      tcp->remote_host, host, tcp->port);
			if (connect(sd, sa, len) && check_error(sd, tcp->timeout)) {
			next:
				tries++;

				ai = ai->ai_next;
				if (ai) {
					logit(LOG_INFO, "Failed connecting to that server: %s",
					      errno != EINPROGRESS ? strerror(errno) : "retrying ...");
					close(sd);
					continue;
				}

				logit(LOG_WARNING, "Failed connecting to %s: %s", tcp->remote_host, strerror(errno));
				close(sd);
				rc = RC_TCP_CONNECT_FAILED;
			} else {
				tcp->socket = sd;
				tcp->initialized = 1;
			}

			break;
		}

		freeaddrinfo(servinfo);
	}
	while (0);

	if (rc) {
		tcp_exit(tcp);
		return rc;
	}

	return 0;
}

int tcp_exit(tcp_sock_t *tcp)
{
	ASSERT(tcp);

	if (!tcp->initialized)
		return 0;

	if (tcp->socket > -1) {
		close(tcp->socket);
		tcp->socket = -1;
	}

	tcp->initialized = 0;

	return 0;
}

int tcp_send(tcp_sock_t *tcp, const char *buf, int len)
{
	ASSERT(tcp);

	if (!tcp->initialized)
		return RC_TCP_OBJECT_NOT_INITIALIZED;
again:
	if (send(tcp->socket, buf, len, 0) == -1) {
		int err = errno;
		if(err == EAGAIN || err == EWOULDBLOCK){
			goto again;
		}
		logit(LOG_WARNING, "Network error while sending query/update: %s", strerror(errno));
		return RC_TCP_SEND_ERROR;
	}

	return 0;
}

int tcp_recv(tcp_sock_t *tcp, char *buf, int len, int *recv_len)
{
	int rc = 0;
	int remaining_bytes = len;
	int total_bytes = 0;

	ASSERT(tcp);
	ASSERT(buf);
	ASSERT(recv_len);

	if (!tcp->initialized)
		return RC_TCP_OBJECT_NOT_INITIALIZED;

	while (remaining_bytes > 0) {
		int bytes;
		int err = 0;
		int chunk_size = remaining_bytes > TCP_DEFAULT_READ_CHUNK_SIZE
			? TCP_DEFAULT_READ_CHUNK_SIZE
			: remaining_bytes;

		bytes = recv(tcp->socket, buf + total_bytes, chunk_size, 0);
		err = errno;
		if(bytes == -1 && (err == EAGAIN || err == EWOULDBLOCK)){
			continue;
		}
		if (bytes < 0) {
			logit(LOG_WARNING, "Network error while waiting for reply: %s", strerror(errno));
			rc = RC_TCP_RECV_ERROR;
			break;
		}

		if (bytes == 0) {
			if (total_bytes == 0)
				rc = RC_TCP_RECV_ERROR;
			break;
		}

		total_bytes    += bytes;
		remaining_bytes = len - total_bytes;
	}

	*recv_len = total_bytes;

	return rc;
}

int tcp_set_port(tcp_sock_t *tcp, int port)
{
	ASSERT(tcp);

	if (port < 0 || port > TCP_SOCKET_MAX_PORT)
		return RC_TCP_BAD_PARAMETER;

	tcp->port = port;

	return 0;
}

int tcp_get_port(tcp_sock_t *tcp, int *port)
{
	ASSERT(tcp);
	ASSERT(port);
	*port = tcp->port;

	return 0;
}

int tcp_set_remote_name(tcp_sock_t *tcp, const char *name)
{
	ASSERT(tcp);
	tcp->remote_host = name;

	return 0;
}

int tcp_get_remote_name(tcp_sock_t *tcp, const char **name)
{
	ASSERT(tcp);
	ASSERT(name);
	*name = tcp->remote_host;

	return 0;
}

int tcp_set_remote_timeout(tcp_sock_t *tcp, int timeout)
{
	ASSERT(tcp);
	tcp->timeout = timeout;

	return 0;
}

int tcp_get_remote_timeout(tcp_sock_t *tcp, int *timeout)
{
	ASSERT(tcp);
	ASSERT(timeout);
	*timeout = tcp->timeout;

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
