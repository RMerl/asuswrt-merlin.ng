/*
 * network server/client for ALSA sequencer
 *   ver.0.1
 *
 * Copyright (C) 1999-2000 Takashi Iwai
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <locale.h>
#include <alsa/asoundlib.h>
#include <getopt.h>
#include <signal.h>
#include <assert.h>
#include "aconfig.h"
#include "gettext.h"

/*
 * prototypes
 */
static void usage(void);
static void init_buf(void);
static void init_pollfds(void);
static void close_files(void);
static void init_seq(char *source, char *dest);
static int get_port(char *service);
static void sigterm_exit(int sig);
static void init_server(int port);
static void init_client(char *server, int port);
static void do_loop(void);
static int copy_local_to_remote(void);
static int copy_remote_to_local(int fd);

/*
 * default TCP port number
 */
#define DEFAULT_PORT	40002

/*
 * local input buffer
 */
static char *readbuf;
static int max_rdlen;
static char *writebuf;
static int cur_wrlen, max_wrlen;

#define MAX_BUF_EVENTS	200
#define MAX_CONNECTION	10

static snd_seq_t *handle;
static struct pollfd *seqifds = NULL;
static struct pollfd *seqofds = NULL;
static struct pollfd *pollfds = NULL;
static int seqifds_count = 0;
static int seqofds_count = 0;
static int pollfds_count = 0;
static int sockfd, netfd[MAX_CONNECTION] = {[0 ... MAX_CONNECTION-1] = -1};
static int max_connection;
static int cur_connected;
static int seq_port;

static int server_mode;
static int verbose = 0;
static int info = 0;


/*
 * main routine
 */

static const struct option long_option[] = {
	{"port", 1, NULL, 'p'},
	{"source", 1, NULL, 's'},
	{"dest", 1, NULL, 'd'},
	{"help", 0, NULL, 'h'},
	{"verbose", 0, NULL, 'v'},
	{"info", 0, NULL, 'i'},
	{NULL, 0, NULL, 0},
};

int main(int argc, char **argv)
{
	int c;
	int port = DEFAULT_PORT;
	char *source = NULL, *dest = NULL;

#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	textdomain(PACKAGE);
#endif

	while ((c = getopt_long(argc, argv, "p:s:d:vi", long_option, NULL)) != -1) {
		switch (c) {
		case 'p':
			if (isdigit(*optarg))
				port = atoi(optarg);
			else
				port = get_port(optarg);
			break;
		case 's':
			source = optarg;
			break;
		case 'd':
			dest = optarg;
			break;
		case 'v':
			verbose++;
			break;
		case 'i':
			info++;
			break;
		default:
			usage();
			exit(1);
		}
	}

	signal(SIGINT, sigterm_exit);
	signal(SIGTERM, sigterm_exit);

	init_buf();
	init_seq(source, dest);

	if (optind >= argc) {
		server_mode = 1;
		max_connection = MAX_CONNECTION;
		init_pollfds();
		init_server(port);
	} else {
		server_mode = 0;
		max_connection = 1;
		init_pollfds();
		init_client(argv[optind], port);
	}

	do_loop();

	close_files();

	return 0;
}


/*
 * print usage
 */
static void usage(void)
{
	printf(_("aseqnet - network client/server on ALSA sequencer\n"));
	printf(_("  Copyright (C) 1999 Takashi Iwai\n"));
	printf(_("usage:\n"));
	printf(_("  server mode: aseqnet [-options]\n"));
	printf(_("  client mode: aseqnet [-options] server_host\n"));
	printf(_("options:\n"));
	printf(_("  -p,--port # : sepcify TCP port (digit or service name)\n"));
	printf(_("  -s,--source addr : read from given addr (client:port)\n"));
	printf(_("  -d,--dest addr : write to given addr (client:port)\n"));
	printf(_("  -v, --verbose : print verbose messages\n"));
	printf(_("  -i, --info : print certain received events\n"));
}


/*
 * allocate and initialize buffers
 */
static void init_buf(void)
{
	max_wrlen = MAX_BUF_EVENTS * sizeof(snd_seq_event_t);
	max_rdlen = MAX_BUF_EVENTS * sizeof(snd_seq_event_t);
	writebuf = malloc(max_wrlen);
	readbuf = malloc(max_rdlen);
	if (writebuf == NULL || readbuf == NULL) {
		fprintf(stderr, _("can't malloc\n"));
		exit(1);
	}
	memset(writebuf, 0, max_wrlen);
	memset(readbuf, 0, max_rdlen);
	cur_wrlen = 0;
}

/*
 * allocate and initialize poll array
 */
static void init_pollfds(void)
{
	pollfds_count = seqifds_count + seqofds_count + 1 + max_connection;
	pollfds = (struct pollfd *)calloc(pollfds_count, sizeof(struct pollfd));
	assert(pollfds);
}

/*
 * close all files
 */
static void close_files(void)
{
	int i;
	if (verbose)
		fprintf(stderr, _("closing files..\n"));
	for (i = 0; i < max_connection; i++) {
		if (netfd[i] >= 0)
			close(netfd[i]);
	}
	if (sockfd >= 0)
		close(sockfd);
}


/*
 * initialize sequencer
 */
static void init_seq(char *source, char *dest)
{
	snd_seq_addr_t addr;
	int err, counti, counto;

	if (snd_seq_open(&handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
		perror("snd_seq_open");
		exit(1);
	}
	if (seqifds)
		free(seqifds);
	if (seqofds)
		free(seqofds);
	counti = seqifds_count = snd_seq_poll_descriptors_count(handle, POLLIN);
	assert(counti > 0);
	counto = seqofds_count = snd_seq_poll_descriptors_count(handle, POLLOUT);
	assert(counto > 0);
	seqifds = (struct pollfd *)calloc(counti, sizeof(struct pollfd));
	assert(seqifds);
	seqofds = (struct pollfd *)calloc(counto, sizeof(struct pollfd));
	assert(seqofds);
	err = snd_seq_poll_descriptors(handle, seqifds, counti, POLLIN);
	assert(err == counti);
	err = snd_seq_poll_descriptors(handle, seqofds, counto, POLLOUT);
	assert(err == counto);

	snd_seq_nonblock(handle, 1);

	/* set client info */
	if (server_mode)
		snd_seq_set_client_name(handle, "Net Server");
	else
		snd_seq_set_client_name(handle, "Net Client");

	/* create a port */
	seq_port = snd_seq_create_simple_port(handle, "Network",
					      SND_SEQ_PORT_CAP_READ |
					      SND_SEQ_PORT_CAP_WRITE |
					      SND_SEQ_PORT_CAP_SUBS_READ |
					      SND_SEQ_PORT_CAP_SUBS_WRITE,
					      SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if (seq_port < 0) {
		perror("create seq port");
		exit(1);
	}
	if (verbose)
		fprintf(stderr, _("sequencer opened: %d:%d\n"),
			snd_seq_client_id(handle), seq_port);

	/* explicit subscriptions */
	if (source) {
		/* read subscription */
		if (snd_seq_parse_address(handle, &addr, source) < 0) {
			fprintf(stderr, _("invalid source address %s\n"), source);
			exit(1);
		}
		if (snd_seq_connect_from(handle, seq_port, addr.client, addr.port)) {
			perror("read subscription");
			exit(1);
		}
	}
	if (dest) {
		/* write subscription */
		if (snd_seq_parse_address(handle, &addr, dest) < 0) {
			fprintf(stderr, _("invalid destination address %s\n"), dest);
			exit(1);
		}
		if (snd_seq_connect_to(handle, seq_port, addr.client, addr.port)) {
			perror("write subscription");
			exit(1);
		}
	}
}


/*
 * convert from string to TCP port number
 */
static int get_port(char *service)
{
	struct servent *sp;

	if ((sp = getservbyname(service, "tcp")) == NULL){
		fprintf(stderr, _("service '%s' is not found in /etc/services\n"), service);
		return -1;
	}
	return sp->s_port;
}

/*
 * signal handler
 */
static void sigterm_exit(int sig)
{
	close_files();
	exit(1);
}


/*
 * initialize network server
 */
static void init_server(int port)
{
	int i;
	int curstate = 1;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)  {
		perror("create socket");
		exit(1);
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &curstate, sizeof(curstate));
	/* the return value is ignored.. */

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)  {
		perror("can't bind");
		exit(1);
	}

	if (listen(sockfd, 5) < 0)  {
		perror("can't listen");
		exit(1);
	}

	cur_connected = 0;
	for (i = 0; i < max_connection; i++)
		netfd[i] = -1;
}

/*
 * start connection on server
 */
static void start_connection(void)
{
	struct sockaddr_in addr;
	int i;
	socklen_t addr_len;

	for (i = 0; i < max_connection; i++) {
		if (netfd[i] < 0)
			break;
	}
	if (i >= max_connection) {
		fprintf(stderr, _("too many connections!\n"));
		exit(1);
	}
	memset(&addr, 0, sizeof(addr));
	addr_len = sizeof(addr);
	netfd[i] = accept(sockfd, (struct sockaddr *)&addr, &addr_len);
	if (netfd[i] < 0) {
		perror("accept");
		exit(1);
	}
	if (verbose)
		fprintf(stderr, _("accepted[%d]\n"), netfd[i]);
	cur_connected++;
}

/*
 * initialize network client
 */
static void init_client(char *server, int port)
{
	struct sockaddr_in addr;
	struct hostent *host;
	int curstate = 1;
	int fd;

	if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("create socket");
		exit(1);
	}
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &curstate, sizeof(curstate)) < 0) {
		perror("setsockopt");
		exit(1);
	}
	if ((host = gethostbyname(server)) == NULL){
		fprintf(stderr, _("can't get address %s\n"), server);
		exit(1);
	}
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	memcpy(&addr.sin_addr, host->h_addr, host->h_length);
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		exit(1);
	}
	if (verbose)
		fprintf(stderr, _("ok.. connected\n"));
	netfd[0] = fd;
	cur_connected = 1;
}

/*
 * event loop
 */
static void do_loop(void)
{
	int i, rc, width;
	int seqifd_ptr, sockfd_ptr = -1, netfd_ptr;

	for (;;) {
		memset(pollfds, 0, pollfds_count * sizeof(struct pollfd));
		seqifd_ptr = 0;
		memcpy(pollfds, seqifds, sizeof(*seqifds)*(width = seqifds_count));
		if (server_mode) {
			sockfd_ptr = width;
			pollfds[width].fd = sockfd;
			pollfds[width].events = POLLIN;
			width++;
		}
		netfd_ptr = width;
		for (i = 0; i < max_connection; i++) {
			if (netfd[i] >= 0) {
				pollfds[width].fd = netfd[i];
				pollfds[width].events = POLLIN;
				width++;
			}
		}
		do {
			rc = poll(pollfds, width, -1);
		} while (rc <= 0 && errno == EINTR);
		if (rc <= 0) {
			perror("poll");
			exit(1);
		}
		if (server_mode) {
			if (pollfds[sockfd_ptr].revents & (POLLIN|POLLOUT))
				start_connection();
		}
		for (i = 0; i < seqifds_count; i++)
			if (pollfds[seqifd_ptr + i].revents & (POLLIN|POLLOUT)) {
				if (copy_local_to_remote())
					return;
				break;
			}
		for (i = 0; i < max_connection; i++) {
			if (netfd[i] < 0)
				continue;
			if (pollfds[netfd_ptr + i].revents & (POLLIN|POLLOUT)) {
				if (copy_remote_to_local(netfd[i])) {
					netfd[i] = -1;
					cur_connected--;
					if (cur_connected <= 0)
						return;
				}
			}
		}
	}
}


/*
 * flush write buffer - send data to the socket
 */
static void flush_writebuf(void)
{
	if (cur_wrlen) {
		int i;
		for (i = 0; i < max_connection; i++) {
			if (netfd[i] >= 0)
				write(netfd[i], writebuf, cur_wrlen);
		}
		cur_wrlen = 0;
	}
}

/*
 * get space from write buffer
 */
static char *get_writebuf(int len)
{
	char *buf;
	if (cur_wrlen + len >= max_wrlen)
		flush_writebuf();
	buf = writebuf + cur_wrlen;
	cur_wrlen += len;
	return buf;
}

static void print_event(snd_seq_event_t *ev)
{
	switch (ev->type) {
	case SND_SEQ_EVENT_CONTROLLER: 
		printf(_("Channel %2d: Control event : %5d\n"),
			ev->data.control.channel, ev->data.control.value);
		break;
	case SND_SEQ_EVENT_PITCHBEND:
		printf(_("Channel %2d: Pitchbender   : %5d\n"), 
			ev->data.control.channel, ev->data.control.value);
		break;
	case SND_SEQ_EVENT_NOTEON:
		printf(_("Channel %2d: Note On event : %5d\n"),
			ev->data.control.channel, ev->data.note.note);
		break;
	case SND_SEQ_EVENT_NOTEOFF: 
		printf(_("Channel %2d: Note Off event: %5d\n"),
		       ev->data.control.channel, ev->data.note.note);           
		break;
	}
}

#define EVENT_PACKET_SIZE	32

/*
 * copy events from sequencer to port(s)
 */
static int copy_local_to_remote(void)
{
	int rc;
	snd_seq_event_t *ev;
	char *buf;

	while ((rc = snd_seq_event_input(handle, &ev)) >= 0 && ev) {
		if (ev->type >= SND_SEQ_EVENT_CLIENT_START &&
		    ! snd_seq_ev_is_variable_type(ev)) {
			snd_seq_free_event(ev);
			continue;
		}
		if (snd_seq_ev_is_variable(ev)) {
			int len;
			len = EVENT_PACKET_SIZE + ev->data.ext.len;
			buf = get_writebuf(len);
			memcpy(buf, ev, sizeof(snd_seq_event_t));
			memcpy(buf + EVENT_PACKET_SIZE, ev->data.ext.ptr, ev->data.ext.len);
		} else {
			buf = get_writebuf(EVENT_PACKET_SIZE);
			memcpy(buf, ev, EVENT_PACKET_SIZE);
		}
		if (info)
			print_event(ev);
		snd_seq_free_event(ev);
	}
	flush_writebuf();
	return 0;
}

/*
 * copy events from a port to sequencer
 */
static int copy_remote_to_local(int fd)
{
	int count;
	char *buf;
	snd_seq_event_t *ev;

	count = read(fd, readbuf, MAX_BUF_EVENTS * sizeof(snd_seq_event_t));
	buf = readbuf;

	if (count == 0) {
		if (verbose)
			fprintf(stderr, _("disconnected\n"));
		return 1;
	}

	while (count > 0) {
		ev = (snd_seq_event_t*)buf;
		buf += EVENT_PACKET_SIZE;
		count -= EVENT_PACKET_SIZE;
		if (snd_seq_ev_is_variable(ev) && ev->data.ext.len > 0) {
			ev->data.ext.ptr = buf;
			buf += ev->data.ext.len;
			count -= ev->data.ext.len;
		}
		snd_seq_ev_set_direct(ev);
		snd_seq_ev_set_source(ev, seq_port);
		snd_seq_ev_set_subs(ev);
		if (info)
			print_event(ev);
		snd_seq_event_output(handle, ev);
	}

	snd_seq_drain_output(handle);
	return 0;
}
