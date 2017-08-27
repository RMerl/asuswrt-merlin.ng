/*
 *  Sequencer Interface - middle-level routines
 *
 *  Copyright (c) 1999 by Takashi Iwai <tiwai@suse.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include "seq_local.h"

/**
 * \brief queue controls - start/stop/continue
 * \param seq sequencer handle
 * \param q queue id to control
 * \param type event type
 * \param value event value
 * \param ev event instance
 *
 * This function sets up general queue control event and sends it.
 * To send at scheduled time, set the schedule in \a ev.
 * If \a ev is NULL, the event is composed locally and sent immediately
 * to the specified queue.  In any cases, you need to call #snd_seq_drain_output()
 * appropriately to feed the event.
 *
 * \sa snd_seq_alloc_queue()
 */
int snd_seq_control_queue(snd_seq_t *seq, int q, int type, int value, snd_seq_event_t *ev)
{
	snd_seq_event_t tmpev;
	if (ev == NULL) {
		snd_seq_ev_clear(&tmpev);
		ev = &tmpev;
		snd_seq_ev_set_direct(ev);
	}
	snd_seq_ev_set_queue_control(ev, type, q, value);
	return snd_seq_event_output(seq, ev);
}


/**
 * \brief create a port - simple version
 * \param seq sequencer handle
 * \param name the name of the port
 * \param caps capability bits
 * \param type type bits
 * \return the created port number or negative error code
 *
 * Creates a port with the given capability and type bits.
 *
 * \sa snd_seq_create_port(), snd_seq_delete_simple_port()
 */
int snd_seq_create_simple_port(snd_seq_t *seq, const char *name,
			       unsigned int caps, unsigned int type)
{
	snd_seq_port_info_t pinfo;
	int result;

	memset(&pinfo, 0, sizeof(pinfo));
	if (name)
		strncpy(pinfo.name, name, sizeof(pinfo.name) - 1);
	pinfo.capability = caps;
	pinfo.type = type;
	pinfo.midi_channels = 16;
	pinfo.midi_voices = 64;
	pinfo.synth_voices = 0;

	result = snd_seq_create_port(seq, &pinfo);
	if (result < 0)
		return result;
	else
		return pinfo.addr.port;
}

/**
 * \brief delete the port
 * \param seq sequencer handle
 * \param port port id
 * \return 0 on success or negative error code
 *
 * \sa snd_seq_delete_port(), snd_seq_create_simple_port()
 */
int snd_seq_delete_simple_port(snd_seq_t *seq, int port)
{
	return snd_seq_delete_port(seq, port);
}

/**
 * \brief simple subscription (w/o exclusive & time conversion)
 * \param seq sequencer handle
 * \param myport the port id as receiver
 * \param src_client sender client id
 * \param src_port sender port id
 * \return 0 on success or negative error code
 *
 * Connect from the given sender client:port to the given destination port in the
 * current client.
 *
 * \sa snd_seq_subscribe_port(), snd_seq_disconnect_from()
 */
int snd_seq_connect_from(snd_seq_t *seq, int myport, int src_client, int src_port)
{
	snd_seq_port_subscribe_t subs;
	
	memset(&subs, 0, sizeof(subs));
	subs.sender.client = src_client;
	subs.sender.port = src_port;
	/*subs.dest.client = seq->client;*/
	subs.dest.client = snd_seq_client_id(seq);
	subs.dest.port = myport;

	return snd_seq_subscribe_port(seq, &subs);
}

/**
 * \brief simple subscription (w/o exclusive & time conversion)
 * \param seq sequencer handle
 * \param myport the port id as sender
 * \param dest_client destination client id
 * \param dest_port destination port id
 * \return 0 on success or negative error code
 *
 * Connect from the given receiver port in the current client
 * to the given destination client:port.
 *
 * \sa snd_seq_subscribe_port(), snd_seq_disconnect_to()
 */
int snd_seq_connect_to(snd_seq_t *seq, int myport, int dest_client, int dest_port)
{
	snd_seq_port_subscribe_t subs;
	
	memset(&subs, 0, sizeof(subs));
	/*subs.sender.client = seq->client;*/
	subs.sender.client = snd_seq_client_id(seq);
	subs.sender.port = myport;
	subs.dest.client = dest_client;
	subs.dest.port = dest_port;

	return snd_seq_subscribe_port(seq, &subs);
}

/**
 * \brief simple disconnection
 * \param seq sequencer handle
 * \param myport the port id as receiver
 * \param src_client sender client id
 * \param src_port sender port id
 * \return 0 on success or negative error code
 *
 * Remove connection from the given sender client:port
 * to the given destination port in the current client.
 *
 * \sa snd_seq_unsubscribe_port(), snd_seq_connect_from()
 */
int snd_seq_disconnect_from(snd_seq_t *seq, int myport, int src_client, int src_port)
{
	snd_seq_port_subscribe_t subs;
	
	memset(&subs, 0, sizeof(subs));
	subs.sender.client = src_client;
	subs.sender.port = src_port;
	/*subs.dest.client = seq->client;*/
	subs.dest.client = snd_seq_client_id(seq);
	subs.dest.port = myport;

	return snd_seq_unsubscribe_port(seq, &subs);
}

/**
 * \brief simple disconnection
 * \param seq sequencer handle
 * \param myport the port id as sender
 * \param dest_client destination client id
 * \param dest_port destination port id
 * \return 0 on success or negative error code
 *
 * Remove connection from the given sender client:port
 * to the given destination port in the current client.
 *
 * \sa snd_seq_unsubscribe_port(), snd_seq_connect_to()
 */
int snd_seq_disconnect_to(snd_seq_t *seq, int myport, int dest_client, int dest_port)
{
	snd_seq_port_subscribe_t subs;
	
	memset(&subs, 0, sizeof(subs));
	/*subs.sender.client = seq->client;*/
	subs.sender.client = snd_seq_client_id(seq);
	subs.sender.port = myport;
	subs.dest.client = dest_client;
	subs.dest.port = dest_port;

	return snd_seq_unsubscribe_port(seq, &subs);
}

/*
 * set client information
 */

/**
 * \brief set client name
 * \param seq sequencer handle
 * \param name name string
 * \return 0 on success or negative error code
 *
 * \sa snd_seq_set_client_info()
 */
int snd_seq_set_client_name(snd_seq_t *seq, const char *name)
{
	snd_seq_client_info_t info;
	int err;

	if ((err = snd_seq_get_client_info(seq, &info)) < 0)
		return err;
	strncpy(info.name, name, sizeof(info.name) - 1);
	return snd_seq_set_client_info(seq, &info);
}

/**
 * \brief add client event filter
 * \param seq sequencer handle
 * \param event_type event type to be added
 * \return 0 on success or negative error code
 *
 * \sa snd_seq_set_client_info()
 */
int snd_seq_set_client_event_filter(snd_seq_t *seq, int event_type)
{
	snd_seq_client_info_t info;
	int err;

	if ((err = snd_seq_get_client_info(seq, &info)) < 0)
		return err;
	snd_seq_client_info_event_filter_add(&info, event_type);
	return snd_seq_set_client_info(seq, &info);
}

/**
 * \brief change the output pool size of the given client
 * \param seq sequencer handle
 * \param size output pool size
 * \return 0 on success or negative error code
 *
 * \sa snd_seq_set_client_pool()
 */
int snd_seq_set_client_pool_output(snd_seq_t *seq, size_t size)
{
	snd_seq_client_pool_t info;
	int err;

	if ((err = snd_seq_get_client_pool(seq, &info)) < 0)
		return err;
	info.output_pool = size;
	return snd_seq_set_client_pool(seq, &info);
}

/**
 * \brief change the output room size of the given client
 * \param seq sequencer handle
 * \param size output room size
 * \return 0 on success or negative error code
 *
 * \sa snd_seq_set_client_pool()
 */
int snd_seq_set_client_pool_output_room(snd_seq_t *seq, size_t size)
{
	snd_seq_client_pool_t info;
	int err;

	if ((err = snd_seq_get_client_pool(seq, &info)) < 0)
		return err;
	info.output_room = size;
	return snd_seq_set_client_pool(seq, &info);
}

/**
 * \brief change the input pool size of the given client
 * \param seq sequencer handle
 * \param size input pool size
 * \return 0 on success or negative error code
 *
 * \sa snd_seq_set_client_pool()
 */
int snd_seq_set_client_pool_input(snd_seq_t *seq, size_t size)
{
	snd_seq_client_pool_t info;
	int err;

	if ((err = snd_seq_get_client_pool(seq, &info)) < 0)
		return err;
	info.input_pool = size;
	return snd_seq_set_client_pool(seq, &info);
}

/**
 * \brief reset client output pool
 * \param seq sequencer handle
 * \return 0 on success or negative error code
 *
 * So far, this works identically like #snd_seq_drop_output().
 */
int snd_seq_reset_pool_output(snd_seq_t *seq)
{
	return snd_seq_drop_output(seq);
}

/**
 * \brief reset client input pool
 * \param seq sequencer handle
 * \return 0 on success or negative error code
 *
 * So far, this works identically like #snd_seq_drop_input().
 */
int snd_seq_reset_pool_input(snd_seq_t *seq)
{
	return snd_seq_drop_input(seq);
}

/**
 * \brief wait until all events are processed
 * \param seq sequencer handle
 * \return 0 on success or negative error code
 *
 * This function waits until all events of this client are processed.
 *
 * \sa snd_seq_drain_output()
 */
int snd_seq_sync_output_queue(snd_seq_t *seq)
{
	int err;
	snd_seq_client_pool_t info;
	int saved_room;
	struct pollfd pfd;

	assert(seq);
	/* reprogram the room size to full */
	if ((err = snd_seq_get_client_pool(seq, &info)) < 0)
		return err;
	saved_room = info.output_room;
	info.output_room = info.output_pool; /* wait until all gone */
	if ((err = snd_seq_set_client_pool(seq, &info)) < 0)
		return err;
	/* wait until all events are purged */
	pfd.fd = seq->poll_fd;
	pfd.events = POLLOUT;
	err = poll(&pfd, 1, -1);
	/* restore the room size */ 
	info.output_room = saved_room;
	snd_seq_set_client_pool(seq, &info);
	return err;
}

/**
 * \brief parse the given string and get the sequencer address
 * \param seq sequencer handle
 * \param addr the address pointer to be returned
 * \param arg the string to be parsed
 * \return 0 on success or negative error code
 *
 * This function parses the sequencer client and port numbers from the given string.
 * The client and port tokes are separated by either colon or period, e.g. 128:1.
 * When \a seq is not NULL, the function accepts also a client name not only
 * digit numbers.
 */
int snd_seq_parse_address(snd_seq_t *seq, snd_seq_addr_t *addr, const char *arg)
{
	char *p;
	int client, port;
	int len;

	assert(addr && arg);

	if ((p = strpbrk(arg, ":.")) != NULL) {
		if ((port = atoi(p + 1)) < 0)
			return -EINVAL;
		len = (int)(p - arg); /* length of client name */
	} else {
		port = 0;
		len = strlen(arg);
	}
	addr->port = port;
	if (isdigit(*arg)) {
		client = atoi(arg);
		if (client < 0)
			return -EINVAL;
		addr->client = client;
	} else {
		/* convert from the name */
		snd_seq_client_info_t cinfo;

		if (! seq)
			return -EINVAL;
		if (len <= 0)
			return -EINVAL;
		cinfo.client = -1;
		while (snd_seq_query_next_client(seq, &cinfo) >= 0) {
			if (! strncmp(arg, cinfo.name, len)) {
				addr->client = cinfo.client;
				return 0;
			}
		}
		return -ENOENT; /* not found */
	}
	return 0;
}
