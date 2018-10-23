/*
 * (C) 2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>

#include "channel.h"
#include "network.h"

struct multichannel *
multichannel_open(struct channel_conf *conf, int len)
{
	struct multichannel *m;
	int i, set_default_channel = 0;

	if (len <= 0 || len > MULTICHANNEL_MAX)
		return NULL;

	m = calloc(sizeof(struct multichannel), 1);
	if (m == NULL)
		return NULL;

	m->channel_num = len;
	for (i = 0; i < len; i++) {
		m->channel[i] = channel_open(&conf[i]);
		if (m->channel[i] == NULL) {
			int j;

			for (j=0; j<i; j++) {
				channel_close(m->channel[j]);
			}
			free(m);
			return NULL;
		}
		if (conf[i].channel_flags & CHANNEL_F_DEFAULT) {
			m->current = m->channel[i];
			set_default_channel = 1;
		}
	}
	if (!set_default_channel)
		m->current = m->channel[0];

	return m;
}

int multichannel_send(struct multichannel *c, const struct nethdr *net)
{
	return channel_send(c->current, net);
}

int multichannel_send_flush(struct multichannel *c)
{
	return channel_send_flush(c->current);
}

int multichannel_recv(struct multichannel *c, char *buf, int size)
{
	return channel_recv(c->current, buf, size);
}

void multichannel_close(struct multichannel *m)
{
	int i;

	for (i = 0; i < m->channel_num; i++) {
		channel_close(m->channel[i]);
	}
	free(m);
}

void multichannel_stats(struct multichannel *m, int fd)
{
	channel_stats(m->current, fd);
}

void
multichannel_stats_extended(struct multichannel *m,
			    struct nlif_handle *h, int fd)
{
	int i, active;

	for (i = 0; i < m->channel_num; i++) {
		if (m->current == m->channel[i]) {
			active = 1;
		} else {
			active = 0;
		}
		channel_stats_extended(m->channel[i], active, h, fd);
	}
}

int multichannel_get_ifindex(struct multichannel *m, int i)
{
	return m->channel[i]->channel_ifindex;
}

int multichannel_get_current_ifindex(struct multichannel *m)
{
	return m->current->channel_ifindex;
}

void multichannel_set_current_channel(struct multichannel *m, int i)
{
	m->current = m->channel[i];
}

void
multichannel_change_current_channel(struct multichannel *m, struct channel *c)
{
	if (m->current != c)
		m->current = c;
}
