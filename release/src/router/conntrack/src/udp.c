/*
 * (C) 2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "udp.h"

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <limits.h>

struct udp_sock *udp_server_create(struct udp_conf *conf)
{
	int yes = 1;
	struct udp_sock *m;
	socklen_t socklen = sizeof(int);

	m = calloc(sizeof(struct udp_sock), 1);
	if (m == NULL)
		return NULL;

	switch(conf->ipproto) {
	case AF_INET:
	        m->addr.ipv4.sin_family = AF_INET;
	        m->addr.ipv4.sin_port = htons(conf->port);
	        m->addr.ipv4.sin_addr = conf->server.ipv4.inet_addr;
		m->sockaddr_len = sizeof(struct sockaddr_in); 
		break;

	case AF_INET6:
		m->addr.ipv6.sin6_family = AF_INET6;
		m->addr.ipv6.sin6_port = htons(conf->port);
		m->addr.ipv6.sin6_addr = conf->server.ipv6.inet_addr6;
		m->addr.ipv6.sin6_scope_id = conf->server.ipv6.scope_id;
		m->sockaddr_len = sizeof(struct sockaddr_in6);
		break;
	}

	m->fd = socket(conf->ipproto, SOCK_DGRAM, 0);
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

#ifndef SO_RCVBUFFORCE
#define SO_RCVBUFFORCE 33
#endif

	if (conf->rcvbuf &&
	    setsockopt(m->fd, SOL_SOCKET, SO_RCVBUFFORCE, &conf->rcvbuf,
				sizeof(int)) == -1) {
		/* not supported in linux kernel < 2.6.14 */
		if (errno != ENOPROTOOPT) {
			close(m->fd);
			free(m);
			return NULL;
		}
	}

	getsockopt(m->fd, SOL_SOCKET, SO_RCVBUF, &conf->rcvbuf, &socklen);

	if (bind(m->fd, (struct sockaddr *) &m->addr, m->sockaddr_len) == -1) {
		close(m->fd);
		free(m);
		return NULL;
	}

	return m;
}

void udp_server_destroy(struct udp_sock *m)
{
	close(m->fd);
	free(m);
}

struct udp_sock *udp_client_create(struct udp_conf *conf)
{
	int ret = 0;
	struct udp_sock *m;
	socklen_t socklen = sizeof(int);

	m = calloc(sizeof(struct udp_sock), 1);
	if (m == NULL)
		return NULL;

	m->fd = socket(conf->ipproto, SOCK_DGRAM, 0);
	if (m->fd == -1) {
		free(m);
		return NULL;
	}

	if (setsockopt(m->fd, SOL_SOCKET, SO_NO_CHECK, &conf->checksum, 
				sizeof(int)) == -1) {
		close(m->fd);
		free(m);
		return NULL;
	}

#ifndef SO_SNDBUFFORCE
#define SO_SNDBUFFORCE 32
#endif

	if (conf->sndbuf &&
	    setsockopt(m->fd, SOL_SOCKET, SO_SNDBUFFORCE, &conf->sndbuf,
				sizeof(int)) == -1) {
		/* not supported in linux kernel < 2.6.14 */
		if (errno != ENOPROTOOPT) {
			close(m->fd);
			free(m);
			return NULL;
		}
	}

	getsockopt(m->fd, SOL_SOCKET, SO_SNDBUF, &conf->sndbuf, &socklen);

	switch(conf->ipproto) {
	case AF_INET:
		m->addr.ipv4.sin_family = AF_INET;
		m->addr.ipv4.sin_port = htons(conf->port);
		m->addr.ipv4.sin_addr = conf->client.inet_addr;
		m->sockaddr_len = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
		m->addr.ipv6.sin6_family = AF_INET6;
		m->addr.ipv6.sin6_port = htons(conf->port);
		memcpy(&m->addr.ipv6.sin6_addr, &conf->client.inet_addr6,
		       sizeof(struct in6_addr));
		m->sockaddr_len = sizeof(struct sockaddr_in6);
		/* Bind the sender side to the same interface that we use to
		 * receive sync messages.
		 */
		m->addr.ipv6.sin6_scope_id = conf->server.ipv6.scope_id;
		break;
	default:
		ret = -1;
		break;
	}

	if (ret == -1) {
		close(m->fd);
		free(m);
		m = NULL;
	}

	return m;
}

void udp_client_destroy(struct udp_sock *m)
{
	close(m->fd);
	free(m);
}

ssize_t udp_send(struct udp_sock *m, const void *data, int size)
{
	ssize_t ret;
	
	ret = sendto(m->fd, 
		     data,
		     size,
		     0,
		     (struct sockaddr *) &m->addr,
		     m->sockaddr_len);
	if (ret == -1) {
		m->stats.error++;
		return ret;
	}

	m->stats.bytes += ret;
	m->stats.messages++;  

	return ret;
}

ssize_t udp_recv(struct udp_sock *m, void *data, int size)
{
	ssize_t ret;
	socklen_t sin_size = sizeof(struct sockaddr_in);

        ret = recvfrom(m->fd,
		       data, 
		       size,
		       0,
		       (struct sockaddr *)&m->addr,
		       &sin_size);
	if (ret == -1) {
		if (errno != EAGAIN)
			m->stats.error++;
		return ret;
	}

	m->stats.bytes += ret;
	m->stats.messages++;

	return ret;
}

int udp_get_fd(struct udp_sock *m)
{
	return m->fd;
}

int udp_isset(struct udp_sock *m, fd_set *readfds)
{
	return FD_ISSET(m->fd, readfds);
}

int
udp_snprintf_stats(char *buf, size_t buflen, char *ifname,
		   struct udp_stats *s, struct udp_stats *r)
{
	size_t size;

	size = snprintf(buf, buflen, "UDP traffic (active device=%s):\n"
				     "%20llu Bytes sent "
				     "%20llu Bytes recv\n"
				     "%20llu Pckts sent "
				     "%20llu Pckts recv\n"
				     "%20llu Error send "
				     "%20llu Error recv\n\n",
				     ifname,
				     (unsigned long long)s->bytes,
				     (unsigned long long)r->bytes,
				     (unsigned long long)s->messages,
				     (unsigned long long)r->messages,
				     (unsigned long long)s->error,
				     (unsigned long long)r->error);
	return size;
}

int
udp_snprintf_stats2(char *buf, size_t buflen, const char *ifname, 
		    const char *status, int active,
		    struct udp_stats *s, struct udp_stats *r)
{
	size_t size;

	size = snprintf(buf, buflen, 
			"UDP traffic device=%s status=%s role=%s:\n"
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
