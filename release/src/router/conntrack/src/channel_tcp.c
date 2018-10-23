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

#include <stdlib.h>
#include <libnfnetlink/libnfnetlink.h>

#include "channel.h"
#include "tcp.h"

static void
*channel_tcp_open(void *conf)
{
	struct tcp_channel *m;
	struct tcp_conf *c = conf;

	m = calloc(sizeof(struct tcp_channel), 1);
	if (m == NULL)
		return NULL;

	m->client = tcp_client_create(c);
	if (m->client == NULL) {
		free(m);
		return NULL;
	}

	m->server = tcp_server_create(c);
	if (m->server == NULL) {
		tcp_client_destroy(m->client);
		free(m);
		return NULL;
	}
	return m;
}

static int
channel_tcp_send(void *channel, const void *data, int len)
{
	struct tcp_channel *m = channel;
	return tcp_send(m->client, data, len);
}

static int
channel_tcp_recv(void *channel, char *buf, int size)
{
	struct tcp_channel *m = channel;
	return tcp_recv(m->server, buf, size);
}

static void
channel_tcp_close(void *channel)
{
	struct tcp_channel *m = channel;
	tcp_client_destroy(m->client);
	tcp_server_destroy(m->server);
	free(m);
}

static int
channel_tcp_get_fd(void *channel)
{
	struct tcp_channel *m = channel;
	return tcp_get_fd(m->server);
}

static void
channel_tcp_stats(struct channel *c, int fd)
{
	struct tcp_channel *m = c->data;
	char ifname[IFNAMSIZ], buf[512];
	int size;

	if_indextoname(c->channel_ifindex, ifname);
	size = tcp_snprintf_stats(buf, sizeof(buf), ifname,
				  m->client, m->server);
	send(fd, buf, size, 0);
}

static void
channel_tcp_stats_extended(struct channel *c, int active,
			   struct nlif_handle *h, int fd)
{
	struct tcp_channel *m = c->data;
	char ifname[IFNAMSIZ], buf[512];
	const char *status;
	unsigned int flags;
	int size;

	if_indextoname(c->channel_ifindex, ifname);
	nlif_get_ifflags(h, c->channel_ifindex, &flags);
	/* 
	 * IFF_UP shows administrative status
	 * IFF_RUNNING shows carrier status
	 */
	if (flags & IFF_UP) {
		if (!(flags & IFF_RUNNING))
			status = "NO-CARRIER";
		else
			status = "RUNNING";
	} else {
		status = "DOWN";
	}
	size = tcp_snprintf_stats2(buf, sizeof(buf),
				     ifname, status, active,
				     &m->client->stats,
				     &m->server->stats);
	send(fd, buf, size, 0);
}

static int
channel_tcp_isset(struct channel *c, fd_set *readfds)
{
	struct tcp_channel *m = c->data;
	return tcp_isset(m->server, readfds);
}

static int
channel_tcp_accept_isset(struct channel *c, fd_set *readfds)
{
	struct tcp_channel *m = c->data;
	return tcp_accept_isset(m->server, readfds);
}

static int
channel_tcp_accept(struct channel *c)
{
	struct tcp_channel *m = c->data;
	return tcp_accept(m->server);
}

struct channel_ops channel_tcp = {
	.headersiz	= 40, /* IP header (20 bytes) + TCP header 20 (bytes) */
	.type		= CHANNEL_T_STREAM,
	.open		= channel_tcp_open,
	.close		= channel_tcp_close,
	.send		= channel_tcp_send,
	.recv		= channel_tcp_recv,
	.accept		= channel_tcp_accept,
	.get_fd		= channel_tcp_get_fd,
	.isset		= channel_tcp_isset,
	.accept_isset	= channel_tcp_accept_isset,
	.stats		= channel_tcp_stats,
	.stats_extended = channel_tcp_stats_extended,
};
