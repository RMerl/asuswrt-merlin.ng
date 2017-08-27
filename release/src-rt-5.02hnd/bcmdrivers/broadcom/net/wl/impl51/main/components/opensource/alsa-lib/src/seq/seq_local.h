/*
 *  Sequencer Interface - definition of sequencer event handler
 *  Copyright (c) 2000 by Jaroslav Kysela <perex@perex.cz>
 *                        Abramo Bagnara <abramo@alsa-project.org>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#ifndef __SEQ_LOCAL_H
#define __SEQ_LOCAL_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "local.h"

#define SND_SEQ_OBUF_SIZE	(16*1024)	/* default size */
#define SND_SEQ_IBUF_SIZE	500		/* in event_size aligned */
#define DEFAULT_TMPBUF_SIZE	20

typedef struct sndrv_seq_queue_client snd_seq_queue_client_t;


typedef struct {
	int (*close)(snd_seq_t *seq);
	int (*nonblock)(snd_seq_t *seq, int nonblock);
	int (*system_info)(snd_seq_t *seq, snd_seq_system_info_t * info);
	int (*get_client_info)(snd_seq_t *seq, snd_seq_client_info_t * info);
	int (*set_client_info)(snd_seq_t *seq, snd_seq_client_info_t * info);
	int (*create_port)(snd_seq_t *seq, snd_seq_port_info_t * port);
	int (*delete_port)(snd_seq_t *seq, snd_seq_port_info_t * port);
	int (*get_port_info)(snd_seq_t *seq, snd_seq_port_info_t * info);
	int (*set_port_info)(snd_seq_t *seq, snd_seq_port_info_t * info);
	int (*get_port_subscription)(snd_seq_t *seq, snd_seq_port_subscribe_t * sub);
	int (*subscribe_port)(snd_seq_t *seq, snd_seq_port_subscribe_t * sub);
	int (*unsubscribe_port)(snd_seq_t *seq, snd_seq_port_subscribe_t * sub);
	int (*query_port_subscribers)(snd_seq_t *seq, snd_seq_query_subscribe_t * subs);
	int (*get_queue_status)(snd_seq_t *seq, snd_seq_queue_status_t * status);
	int (*get_queue_tempo)(snd_seq_t *seq, snd_seq_queue_tempo_t * tempo);
	int (*set_queue_tempo)(snd_seq_t *seq, snd_seq_queue_tempo_t * tempo);
	int (*get_queue_timer)(snd_seq_t *seq, snd_seq_queue_timer_t * timer);
	int (*set_queue_timer)(snd_seq_t *seq, snd_seq_queue_timer_t * timer);
	int (*get_queue_client)(snd_seq_t *seq, snd_seq_queue_client_t * client);
	int (*set_queue_client)(snd_seq_t *seq, snd_seq_queue_client_t * client);
	int (*create_queue)(snd_seq_t *seq, snd_seq_queue_info_t *info);
	int (*delete_queue)(snd_seq_t *seq, snd_seq_queue_info_t *info);
	int (*get_queue_info)(snd_seq_t *seq, snd_seq_queue_info_t *info);
	int (*set_queue_info)(snd_seq_t *seq, snd_seq_queue_info_t *info);
	int (*get_named_queue)(snd_seq_t *seq, snd_seq_queue_info_t *info);
	ssize_t (*write)(snd_seq_t *seq, void *buf, size_t len);
	ssize_t (*read)(snd_seq_t *seq, void *buf, size_t len);
	int (*remove_events)(snd_seq_t *seq, snd_seq_remove_events_t *rmp);
	int (*get_client_pool)(snd_seq_t *seq, snd_seq_client_pool_t *info);
	int (*set_client_pool)(snd_seq_t *seq, snd_seq_client_pool_t *info);
	int (*query_next_client)(snd_seq_t *seq, snd_seq_client_info_t *info);
	int (*query_next_port)(snd_seq_t *seq, snd_seq_port_info_t *info);
} snd_seq_ops_t;

struct _snd_seq {
	char *name;
	snd_seq_type_t type;
	int streams;
	int mode;
	int poll_fd;
	void *dl_handle;
	const snd_seq_ops_t *ops;
	void *private_data;
	int client;		/* client number */
	/* buffers */
	char *obuf;		/* output buffer */
	size_t obufsize;		/* output buffer size */
	size_t obufused;		/* output buffer used size */
	snd_seq_event_t *ibuf;	/* input buffer */
	size_t ibufptr;		/* current pointer of input buffer */
	size_t ibuflen;		/* queued length */
	size_t ibufsize;		/* input buffer size */
	snd_seq_event_t *tmpbuf;	/* temporary event for extracted event */
	size_t tmpbufsize;		/* size of errbuf */
};

int snd_seq_hw_open(snd_seq_t **handle, const char *name, int streams, int mode);

#endif
