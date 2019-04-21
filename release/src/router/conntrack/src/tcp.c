/*
 * (C) 2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * TCP support has been sponsored by 6WIND <www.6wind.com>.
 */

#include "tcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include "conntrackd.h"
#include "fds.h"

struct tcp_sock *tcp_server_create(struct tcp_conf *c)
{
	int yes = 1;
	struct tcp_sock *m;
	socklen_t socklen = sizeof(int);

	m = calloc(sizeof(struct tcp_sock), 1);
	if (m == NULL)
		return NULL;

	m->conf = c;

	switch(c->ipproto) {
	case AF_INET:
	        m->addr.ipv4.sin_family = AF_INET;
	        m->addr.ipv4.sin_port = htons(c->port);
	        m->addr.ipv4.sin_addr = c->server.ipv4.inet_addr;
		m->sockaddr_len = sizeof(struct sockaddr_in); 
		break;

	case AF_INET6:
		m->addr.ipv6.sin6_family = AF_INET6;
		m->addr.ipv6.sin6_port = htons(c->port);
		m->addr.ipv6.sin6_addr = c->server.ipv6.inet_addr6;
		m->addr.ipv6.sin6_scope_id = c->server.ipv6.scope_id;
		m->sockaddr_len = sizeof(struct sockaddr_in6);
		break;
	}

	m->fd = socket(c->ipproto, SOCK_STREAM, 0);
	if (m->fd == -1) {
		free(m);
		return NULL;
	}

	if (setsockopt(m->fd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
		close(m->fd);
		free(m);
		return NULL;
	}

	if (setsockopt(m->fd, SOL_SOCKET, SO_KEEPALIVE, &yes,
				sizeof(int)) == -1) {
		close(m->fd);
		free(m);
		return NULL;
	}

#ifndef SO_RCVBUFFORCE
#define SO_RCVBUFFORCE 33
#endif

	if (c->rcvbuf &&
	    setsockopt(m->fd, SOL_SOCKET, SO_RCVBUFFORCE, &c->rcvbuf,
				sizeof(int)) == -1) {
		/* not supported in linux kernel < 2.6.14 */
		if (errno != ENOPROTOOPT) {
			close(m->fd);
			free(m);
			return NULL;
		}
	}

	getsockopt(m->fd, SOL_SOCKET, SO_RCVBUF, &c->rcvbuf, &socklen);

	if (bind(m->fd, (struct sockaddr *) &m->addr, m->sockaddr_len) == -1) {
		close(m->fd);
		free(m);
		return NULL;
	}

	if (listen(m->fd, 1) == -1) {
		close(m->fd);
		free(m);
		return NULL;
	}

	if (fcntl(m->fd, F_SETFL, O_NONBLOCK) == -1) {
		close(m->fd);
		free(m);
		return NULL;
	}

	m->state = TCP_SERVER_ACCEPTING;

	return m;
}

void tcp_server_destroy(struct tcp_sock *m)
{
	close(m->fd);
	free(m);
}

static int
tcp_client_init(struct tcp_sock *m, struct tcp_conf *c)
{
	int ret = 0;
	socklen_t socklen = sizeof(int);

	m->fd = socket(c->ipproto, SOCK_STREAM, 0);
	if (m->fd == -1)
		return -1;

	if (setsockopt(m->fd, SOL_SOCKET, SO_NO_CHECK, &c->checksum, 
				sizeof(int)) == -1) {
		close(m->fd);
		return -1;
	}

#ifndef SO_SNDBUFFORCE
#define SO_SNDBUFFORCE 32
#endif

	if (c->sndbuf &&
	    setsockopt(m->fd, SOL_SOCKET, SO_SNDBUFFORCE, &c->sndbuf,
				sizeof(int)) == -1) {
		/* not supported in linux kernel < 2.6.14 */
		if (errno != ENOPROTOOPT) {
			close(m->fd);
			return -1;
		}
	}

	getsockopt(m->fd, SOL_SOCKET, SO_SNDBUF, &c->sndbuf, &socklen);

	switch(c->ipproto) {
	case AF_INET:
		m->addr.ipv4.sin_family = AF_INET;
		m->addr.ipv4.sin_port = htons(c->port);
		m->addr.ipv4.sin_addr = c->client.inet_addr;
		m->sockaddr_len = sizeof(struct sockaddr_in); 
		break;
	case AF_INET6:
		m->addr.ipv6.sin6_family = AF_INET6;
		m->addr.ipv6.sin6_port = htons(c->port);
		memcpy(&m->addr.ipv6.sin6_addr, &c->client.inet_addr6,
		       sizeof(struct in6_addr));
		m->sockaddr_len = sizeof(struct sockaddr_in6); 
		break;
	default:
		ret = -1;
		break;
	}

	if (ret == -1) {
		close(m->fd);
		return -1;
	}

	if (fcntl(m->fd, F_SETFL, O_NONBLOCK) == -1) {
		close(m->fd);
		return -1;
	}

	ret = connect(m->fd, (struct sockaddr *)&m->addr, m->sockaddr_len);
	if (ret == -1) {
		if (errno == EINPROGRESS) {
			/* connection in progress ... */
			m->state = TCP_CLIENT_DISCONNECTED;
		} else if (errno == ECONNREFUSED) {
			/* connection refused. */
			m->state = TCP_CLIENT_DISCONNECTED;
		} else {
			/* unexpected error, give up. */
			close(m->fd);
			return -1;
		}
	} else {
		/* very unlikely at this stage. */
		m->state = TCP_CLIENT_CONNECTED;
	}
	return 0;
}

/* We use this to rate-limit the amount of connect() calls per second. */
static struct alarm_block tcp_connect_alarm;
static void tcp_connect_alarm_cb(struct alarm_block *a, void *data) {}

struct tcp_sock *tcp_client_create(struct tcp_conf *c)
{
	struct tcp_sock *m;

	m = calloc(sizeof(struct tcp_sock), 1);
	if (m == NULL)
		return NULL;

	m->conf = c;

	if (tcp_client_init(m, c) == -1) {
		free(m);
		return NULL;
	}

	init_alarm(&tcp_connect_alarm, NULL, tcp_connect_alarm_cb);

	return m;
}

void tcp_client_destroy(struct tcp_sock *m)
{
	close(m->fd);
	free(m);
}

int tcp_accept(struct tcp_sock *m)
{
	int ret;

	/* we got an attempt to connect but we already have a client? */
	if (m->state != TCP_SERVER_ACCEPTING) {
		/* clear the session and restart ... */
		unregister_fd(m->client_fd, STATE(fds));
		close(m->client_fd);
		m->client_fd = -1;
		m->state = TCP_SERVER_ACCEPTING;
	}

	/* the other peer wants to connect ... */
	ret = accept(m->fd, NULL, NULL);
	if (ret == -1) {
		/* unexpected error: Give us another try. Or we have hit
 		 * -EAGAIN, in that case we remain in the accepting connections
		 * state.
		 */
		m->state = TCP_SERVER_ACCEPTING;
	} else {
		/* the peer finally got connected. */
		if (fcntl(ret, F_SETFL, O_NONBLOCK) == -1) {
			/* close the connection and give us another chance. */
			close(ret);
			return -1;
		}

		m->client_fd = ret;
		m->state = TCP_SERVER_CONNECTED;
	}
	return m->client_fd;
}

#define TCP_CONNECT_TIMEOUT	1

ssize_t tcp_send(struct tcp_sock *m, const void *data, int size)
{
	ssize_t ret = 0;

	switch(m->state) {
	case TCP_CLIENT_DISCONNECTED:
		/* We rate-limit the amount of connect() calls. */
		if (alarm_pending(&tcp_connect_alarm)) {
			ret = -1;
			break;
		}
		add_alarm(&tcp_connect_alarm, TCP_CONNECT_TIMEOUT, 0);
		ret = connect(m->fd, (struct sockaddr *)&m->addr,
			      m->sockaddr_len);
		if (ret == -1) {
			if (errno == EINPROGRESS || errno == EALREADY) {
				/* connection in progress or already trying. */
				m->state = TCP_CLIENT_DISCONNECTED;
			} else if (errno == ECONNREFUSED) {
				/* connection refused. */
				m->state = TCP_CLIENT_DISCONNECTED;
				m->stats.error++;
			} else {
				/* unexpected error, give up. */
				m->state = TCP_CLIENT_DISCONNECTED;
				m->stats.error++;
			}
			break;
		} else {
			/* we got connected :) */
			m->state = TCP_CLIENT_CONNECTED;
		}
	case TCP_CLIENT_CONNECTED:
		ret = sendto(m->fd, data, size, 0,
			     (struct sockaddr *) &m->addr, m->sockaddr_len);
		if (ret == -1) {
			if (errno == EPIPE || errno == ECONNRESET) {
				close(m->fd);
				tcp_client_init(m, m->conf);
				m->state = TCP_CLIENT_DISCONNECTED;
				m->stats.error++;
			} else {
				m->stats.error++;
				return -1;
			}
		}
	}

	if (ret >= 0) {
		m->stats.bytes += ret;
		m->stats.messages++;
	}
	return ret;
}

ssize_t tcp_recv(struct tcp_sock *m, void *data, int size)
{
	ssize_t ret = 0;
	socklen_t sin_size = sizeof(struct sockaddr_in);

	/* we are not connected, skip. */
	if (m->state != TCP_SERVER_CONNECTED)
		return 0;

        ret = recvfrom(m->client_fd, data, size, 0,
		       (struct sockaddr *)&m->addr, &sin_size);
	if (ret == -1) {
		/* the other peer has disconnected... */
		if (errno == ENOTCONN) {
			unregister_fd(m->client_fd, STATE(fds));
			close(m->client_fd);
			m->client_fd = -1;
			m->state = TCP_SERVER_ACCEPTING;
		} else if (errno != EAGAIN) {
			m->stats.error++;
		}
	} else if (ret == 0) {
		/* the other peer has closed the connection... */
		unregister_fd(m->client_fd, STATE(fds));
		close(m->client_fd);
		m->client_fd = -1;
		m->state = TCP_SERVER_ACCEPTING;
	}

	if (ret >= 0) {
		m->stats.bytes += ret;
		m->stats.messages++;
	}
	return ret;
}

int tcp_get_fd(struct tcp_sock *m)
{
	return m->fd;
}

int tcp_isset(struct tcp_sock *m, fd_set *readfds)
{
	return m->client_fd >= 0 ? FD_ISSET(m->client_fd, readfds) : 0;
}

int tcp_accept_isset(struct tcp_sock *m, fd_set *readfds)
{
	return FD_ISSET(m->fd, readfds);
}

int
tcp_snprintf_stats(char *buf, size_t buflen, char *ifname,
		   struct tcp_sock *client, struct tcp_sock *server)
{
	size_t size;
	struct tcp_stats *s = &client->stats, *r = &server->stats;

	size = snprintf(buf, buflen, "TCP traffic (active device=%s) "
				     "server=%s client=%s:\n"
				     "%20llu Bytes sent "
				     "%20llu Bytes recv\n"
				     "%20llu Pckts sent "
				     "%20llu Pckts recv\n"
				     "%20llu Error send "
				     "%20llu Error recv\n\n",
				     ifname,
				     server->state == TCP_SERVER_CONNECTED ?
				     "connected" : "disconnected",
				     client->state == TCP_CLIENT_CONNECTED ?
				     "connected" : "disconnected",
				     (unsigned long long)s->bytes,
				     (unsigned long long)r->bytes,
				     (unsigned long long)s->messages,
				     (unsigned long long)r->messages,
				     (unsigned long long)s->error,
				     (unsigned long long)r->error);
	return size;
}

int
tcp_snprintf_stats2(char *buf, size_t buflen, const char *ifname, 
		    const char *status, int active,
		    struct tcp_stats *s, struct tcp_stats *r)
{
	size_t size;

	size = snprintf(buf, buflen, 
			"TCP traffic device=%s status=%s role=%s:\n"
			"%20llu Bytes sent "
			"%20llu Bytes recv\n"
			"%20llu Pckts sent "
			"%20llu Pckts recv\n"
			"%20llu Error send "
			"%20llu Error recv\n\n",
			ifname, status, active ? "ACTIVE" : "BACKUP",
			(unsigned long long)s->bytes,
			(unsigned long long)r->bytes,
			(unsigned long long)s->messages,
			(unsigned long long)r->messages,
			(unsigned long long)s->error,
			(unsigned long long)r->error);
	return size;
}
