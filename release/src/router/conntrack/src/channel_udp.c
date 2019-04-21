/*
 * (C) 2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <libnfnetlink/libnfnetlink.h>

#include "channel.h"
#include "udp.h"

static void
*channel_udp_open(void *conf)
{
	struct udp_channel *m;
	struct udp_conf *c = conf;

	m = calloc(sizeof(struct udp_channel), 1);
	if (m == NULL)
		return NULL;

	m->client = udp_client_create(c);
	if (m->client == NULL) {
		free(m);
		return NULL;
	}

	m->server = udp_server_create(c);
	if (m->server == NULL) {
		udp_client_destroy(m->client);
		free(m);
		return NULL;
	}
	return m;
}

static int
channel_udp_send(void *channel, const void *data, int len)
{
	struct udp_channel *m = channel;
	return udp_send(m->client, data, len);
}

static int
channel_udp_recv(void *channel, char *buf, int size)
{
	struct udp_channel *m = channel;
	return udp_recv(m->server, buf, size);
}

static void
channel_udp_close(void *channel)
{
	struct udp_channel *m = channel;
	udp_client_destroy(m->client);
	udp_server_destroy(m->server);
	free(m);
}

static int
channel_udp_get_fd(void *channel)
{
	struct udp_channel *m = channel;
	return udp_get_fd(m->server);
}

static void
channel_udp_stats(struct channel *c, int fd)
{
	struct udp_channel *m = c->data;
	char ifname[IFNAMSIZ], buf[512];
	int size;

	if_indextoname(c->channel_ifindex, ifname);
	size = udp_snprintf_stats(buf, sizeof(buf), ifname,
				    &m->client->stats, &m->server->stats);
	send(fd, buf, size, 0);
}

static void
channel_udp_stats_extended(struct channel *c, int active,
			     struct nlif_handle *h, int fd)
{
	struct udp_channel *m = c->data;
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
	size = udp_snprintf_stats2(buf, sizeof(buf),
				     ifname, status, active,
				     &m->client->stats,
				     &m->server->stats);
	send(fd, buf, size, 0);
}

static int
channel_udp_isset(struct channel *c, fd_set *readfds)
{
	struct udp_channel *m = c->data;
	return udp_isset(m->server, readfds);
}

static int
channel_udp_accept_isset(struct channel *c, fd_set *readfds)
{
	return 0;
}

struct channel_ops channel_udp = {
	.headersiz	= 28, /* IP header (20 bytes) + UDP header 8 (bytes) */
	.open		= channel_udp_open,
	.close		= channel_udp_close,
	.send		= channel_udp_send,
	.recv		= channel_udp_recv,
	.get_fd		= channel_udp_get_fd,
	.isset		= channel_udp_isset,
	.accept_isset	= channel_udp_accept_isset,
	.stats		= channel_udp_stats,
	.stats_extended = channel_udp_stats_extended,
};
