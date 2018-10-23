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
#include "mcast.h"

static void
*channel_mcast_open(void *conf)
{
	struct mcast_channel *m;
	struct mcast_conf *c = conf;

	m = calloc(sizeof(struct mcast_channel), 1);
	if (m == NULL)
		return NULL;

	m->client = mcast_client_create(c);
	if (m->client == NULL) {
		free(m);
		return NULL;
	}

	m->server = mcast_server_create(c);
	if (m->server == NULL) {
		mcast_client_destroy(m->client);
		free(m);
		return NULL;
	}
	return m;
}

static int
channel_mcast_send(void *channel, const void *data, int len)
{
	struct mcast_channel *m = channel;
	return mcast_send(m->client, data, len);
}

static int
channel_mcast_recv(void *channel, char *buf, int size)
{
	struct mcast_channel *m = channel;
	return mcast_recv(m->server, buf, size);
}

static void
channel_mcast_close(void *channel)
{
	struct mcast_channel *m = channel;
	mcast_client_destroy(m->client);
	mcast_server_destroy(m->server);
	free(m);
}

static int
channel_mcast_get_fd(void *channel)
{
	struct mcast_channel *m = channel;
	return mcast_get_fd(m->server);
}

static void
channel_mcast_stats(struct channel *c, int fd)
{
	struct mcast_channel *m = c->data;
	char ifname[IFNAMSIZ], buf[512];
	int size;

	if_indextoname(c->channel_ifindex, ifname);
	size = mcast_snprintf_stats(buf, sizeof(buf), ifname,
				    &m->client->stats, &m->server->stats);
	send(fd, buf, size, 0);
}

static void
channel_mcast_stats_extended(struct channel *c, int active,
			     struct nlif_handle *h, int fd)
{
	struct mcast_channel *m = c->data;
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
	size = mcast_snprintf_stats2(buf, sizeof(buf),
				     ifname, status, active,
				     &m->client->stats,
				     &m->server->stats);
	send(fd, buf, size, 0);
}

static int
channel_mcast_isset(struct channel *c, fd_set *readfds)
{
	struct mcast_channel *m = c->data;
	return mcast_isset(m->server, readfds);
}

static int
channel_mcast_accept_isset(struct channel *c, fd_set *readfds)
{
	return 0;
}

struct channel_ops channel_mcast = {
	.headersiz	= 28, /* IP header (20 bytes) + UDP header 8 (bytes) */
	.open		= channel_mcast_open,
	.close		= channel_mcast_close,
	.send		= channel_mcast_send,
	.recv		= channel_mcast_recv,
	.get_fd		= channel_mcast_get_fd,
	.isset		= channel_mcast_isset,
	.accept_isset	= channel_mcast_accept_isset,
	.stats		= channel_mcast_stats,
	.stats_extended = channel_mcast_stats_extended,
};
