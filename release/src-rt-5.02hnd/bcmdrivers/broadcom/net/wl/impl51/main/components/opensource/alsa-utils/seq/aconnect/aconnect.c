/*
 * connect / disconnect two subscriber ports
 *   ver.0.1.3
 *
 * Copyright (C) 1999 Takashi Iwai
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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include "aconfig.h"
#include "gettext.h"

static void error_handler(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
	va_list arg;

	if (err == ENOENT)	/* Ignore those misleading "warnings" */
		return;
	va_start(arg, fmt);
	fprintf(stderr, "ALSA lib %s:%i:(%s) ", file, line, function);
	vfprintf(stderr, fmt, arg);
	if (err)
		fprintf(stderr, ": %s", snd_strerror(err));
	putc('\n', stderr);
	va_end(arg);
}

static void usage(void)
{
	printf(_("aconnect - ALSA sequencer connection manager\n"));
	printf(_("Copyright (C) 1999-2000 Takashi Iwai\n"));
	printf(_("Usage:\n"));
	printf(_(" * Connection/disconnection between two ports\n"));
	printf(_("   aconnect [-options] sender receiver\n"));
	printf(_("     sender, receiver = client:port pair\n"));
	printf(_("     -d,--disconnect     disconnect\n"));
	printf(_("     -e,--exclusive      exclusive connection\n"));
	printf(_("     -r,--real #         convert real-time-stamp on queue\n"));
	printf(_("     -t,--tick #         convert tick-time-stamp on queue\n"));
	printf(_(" * List connected ports (no subscription action)\n"));
	printf(_("   aconnect -i|-o [-options]\n"));
	printf(_("     -i,--input          list input (readable) ports\n"));
	printf(_("     -o,--output         list output (writable) ports\n"));
	printf(_("     -l,--list           list current connections of each port\n"));
	printf(_(" * Remove all exported connections\n"));
	printf(_("     -x, --removeall\n"));
}

/*
 * check permission (capability) of specified port
 */

#define LIST_INPUT	1
#define LIST_OUTPUT	2

#define perm_ok(pinfo,bits) ((snd_seq_port_info_get_capability(pinfo) & (bits)) == (bits))

static int check_permission(snd_seq_port_info_t *pinfo, int perm)
{
	if (perm) {
		if (perm & LIST_INPUT) {
			if (perm_ok(pinfo, SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ))
				goto __ok;
		}
		if (perm & LIST_OUTPUT) {
			if (perm_ok(pinfo, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE))
				goto __ok;
		}
		return 0;
	}
 __ok:
	if (snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_NO_EXPORT)
		return 0;
	return 1;
}

/*
 * list subscribers of specified type
 */
static void list_each_subs(snd_seq_t *seq, snd_seq_query_subscribe_t *subs, int type, const char *msg)
{
	int count = 0;
	snd_seq_query_subscribe_set_type(subs, type);
	snd_seq_query_subscribe_set_index(subs, 0);
	while (snd_seq_query_port_subscribers(seq, subs) >= 0) {
		const snd_seq_addr_t *addr;
		if (count++ == 0)
			printf("\t%s: ", msg);
		else
			printf(", ");
		addr = snd_seq_query_subscribe_get_addr(subs);
		printf("%d:%d", addr->client, addr->port);
		if (snd_seq_query_subscribe_get_exclusive(subs))
			printf("[ex]");
		if (snd_seq_query_subscribe_get_time_update(subs))
			printf("[%s:%d]",
			       (snd_seq_query_subscribe_get_time_real(subs) ? "real" : "tick"),
			       snd_seq_query_subscribe_get_queue(subs));
		snd_seq_query_subscribe_set_index(subs, snd_seq_query_subscribe_get_index(subs) + 1);
	}
	if (count > 0)
		printf("\n");
}

/*
 * list subscribers
 */
static void list_subscribers(snd_seq_t *seq, const snd_seq_addr_t *addr)
{
	snd_seq_query_subscribe_t *subs;
	snd_seq_query_subscribe_alloca(&subs);
	snd_seq_query_subscribe_set_root(subs, addr);
	list_each_subs(seq, subs, SND_SEQ_QUERY_SUBS_READ, _("Connecting To"));
	list_each_subs(seq, subs, SND_SEQ_QUERY_SUBS_WRITE, _("Connected From"));
}

/*
 * search all ports
 */
typedef void (*action_func_t)(snd_seq_t *seq, snd_seq_client_info_t *cinfo, snd_seq_port_info_t *pinfo, int count);

static void do_search_port(snd_seq_t *seq, int perm, action_func_t do_action)
{
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;
	int count;

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(seq, cinfo) >= 0) {
		/* reset query info */
		snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		count = 0;
		while (snd_seq_query_next_port(seq, pinfo) >= 0) {
			if (check_permission(pinfo, perm)) {
				do_action(seq, cinfo, pinfo, count);
				count++;
			}
		}
	}
}


static void print_port(snd_seq_t *seq, snd_seq_client_info_t *cinfo,
		       snd_seq_port_info_t *pinfo, int count)
{
	if (! count) {
		printf(_("client %d: '%s' [type=%s]\n"),
		       snd_seq_client_info_get_client(cinfo),
		       snd_seq_client_info_get_name(cinfo),
		       (snd_seq_client_info_get_type(cinfo) == SND_SEQ_USER_CLIENT ?
			_("user") : _("kernel")));
	}
	printf("  %3d '%-16s'\n",
	       snd_seq_port_info_get_port(pinfo),
	       snd_seq_port_info_get_name(pinfo));
}

static void print_port_and_subs(snd_seq_t *seq, snd_seq_client_info_t *cinfo,
				snd_seq_port_info_t *pinfo, int count)
{
	print_port(seq, cinfo, pinfo, count);
	list_subscribers(seq, snd_seq_port_info_get_addr(pinfo));
}


/*
 * remove all (exported) connections
 */
static void remove_connection(snd_seq_t *seq, snd_seq_client_info_t *cinfo,
			      snd_seq_port_info_t *pinfo, int count)
{
	snd_seq_query_subscribe_t *query;
	snd_seq_port_info_t *port;
	snd_seq_port_subscribe_t *subs;

	snd_seq_query_subscribe_alloca(&query);
	snd_seq_query_subscribe_set_root(query, snd_seq_port_info_get_addr(pinfo));
	snd_seq_query_subscribe_set_type(query, SND_SEQ_QUERY_SUBS_READ);
	snd_seq_query_subscribe_set_index(query, 0);

	snd_seq_port_info_alloca(&port);
	snd_seq_port_subscribe_alloca(&subs);

	while (snd_seq_query_port_subscribers(seq, query) >= 0) {
		const snd_seq_addr_t *sender = snd_seq_query_subscribe_get_root(query);
		const snd_seq_addr_t *dest = snd_seq_query_subscribe_get_addr(query);

		if (snd_seq_get_any_port_info(seq, dest->client, dest->port, port) < 0 ||
		    !(snd_seq_port_info_get_capability(port) & SND_SEQ_PORT_CAP_SUBS_WRITE) ||
		    (snd_seq_port_info_get_capability(port) & SND_SEQ_PORT_CAP_NO_EXPORT)) {
			snd_seq_query_subscribe_set_index(query, snd_seq_query_subscribe_get_index(query) + 1);
			continue;
		}
		snd_seq_port_subscribe_set_queue(subs, snd_seq_query_subscribe_get_queue(query));
		snd_seq_port_subscribe_set_sender(subs, sender);
		snd_seq_port_subscribe_set_dest(subs, dest);
		if (snd_seq_unsubscribe_port(seq, subs) < 0) {
			snd_seq_query_subscribe_set_index(query, snd_seq_query_subscribe_get_index(query) + 1);
		}
	}
}

static void remove_all_connections(snd_seq_t *seq)
{
	do_search_port(seq, 0, remove_connection);
}


/*
 * main..
 */

enum {
	SUBSCRIBE, UNSUBSCRIBE, LIST, REMOVE_ALL
};

static const struct option long_option[] = {
	{"disconnect", 0, NULL, 'd'},
	{"input", 0, NULL, 'i'},
	{"output", 0, NULL, 'o'},
	{"real", 1, NULL, 'r'},
	{"tick", 1, NULL, 't'},
	{"exclusive", 0, NULL, 'e'},
	{"list", 0, NULL, 'l'},
	{"removeall", 0, NULL, 'x'},
	{NULL, 0, NULL, 0},
};

int main(int argc, char **argv)
{
	int c;
	snd_seq_t *seq;
	int queue = 0, convert_time = 0, convert_real = 0, exclusive = 0;
	int command = SUBSCRIBE;
	int list_perm = 0;
	int client;
	int list_subs = 0;
	snd_seq_port_subscribe_t *subs;
	snd_seq_addr_t sender, dest;

#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	textdomain(PACKAGE);
#endif

	while ((c = getopt_long(argc, argv, "dior:t:elx", long_option, NULL)) != -1) {
		switch (c) {
		case 'd':
			command = UNSUBSCRIBE;
			break;
		case 'i':
			command = LIST;
			list_perm |= LIST_INPUT;
			break;
		case 'o':
			command = LIST;
			list_perm |= LIST_OUTPUT;
			break;
		case 'e':
			exclusive = 1;
			break;
		case 'r':
			queue = atoi(optarg);
			convert_time = 1;
			convert_real = 1;
			break;
		case 't':
			queue = atoi(optarg);
			convert_time = 1;
			convert_real = 0;
			break;
		case 'l':
			list_subs = 1;
			break;
		case 'x':
			command = REMOVE_ALL;
			break;
		default:
			usage();
			exit(1);
		}
	}

	if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
		fprintf(stderr, _("can't open sequencer\n"));
		return 1;
	}
	
	snd_lib_error_set_handler(error_handler);

	switch (command) {
	case LIST:
		do_search_port(seq, list_perm,
			       list_subs ? print_port_and_subs : print_port);
		snd_seq_close(seq);
		return 0;
	case REMOVE_ALL:
		remove_all_connections(seq);
		snd_seq_close(seq);
		return 0;
	}

	/* connection or disconnection */

	if (optind + 2 > argc) {
		snd_seq_close(seq);
		usage();
		exit(1);
	}

	if ((client = snd_seq_client_id(seq)) < 0) {
		snd_seq_close(seq);
		fprintf(stderr, _("can't get client id\n"));
		return 1;
	}

	/* set client info */
	if (snd_seq_set_client_name(seq, "ALSA Connector") < 0) {
		snd_seq_close(seq);
		fprintf(stderr, _("can't set client info\n"));
		return 1;
	}

	/* set subscription */
	if (snd_seq_parse_address(seq, &sender, argv[optind]) < 0) {
		snd_seq_close(seq);
		fprintf(stderr, _("invalid sender address %s\n"), argv[optind]);
		return 1;
	}
	if (snd_seq_parse_address(seq, &dest, argv[optind + 1]) < 0) {
		snd_seq_close(seq);
		fprintf(stderr, _("invalid destination address %s\n"), argv[optind + 1]);
		return 1;
	}
	snd_seq_port_subscribe_alloca(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	snd_seq_port_subscribe_set_queue(subs, queue);
	snd_seq_port_subscribe_set_exclusive(subs, exclusive);
	snd_seq_port_subscribe_set_time_update(subs, convert_time);
	snd_seq_port_subscribe_set_time_real(subs, convert_real);

	if (command == UNSUBSCRIBE) {
		if (snd_seq_get_port_subscription(seq, subs) < 0) {
			snd_seq_close(seq);
			fprintf(stderr, _("No subscription is found\n"));
			return 1;
		}
		if (snd_seq_unsubscribe_port(seq, subs) < 0) {
			snd_seq_close(seq);
			fprintf(stderr, _("Disconnection failed (%s)\n"), snd_strerror(errno));
			return 1;
		}
	} else {
		if (snd_seq_get_port_subscription(seq, subs) == 0) {
			snd_seq_close(seq);
			fprintf(stderr, _("Connection is already subscribed\n"));
			return 1;
		}
		if (snd_seq_subscribe_port(seq, subs) < 0) {
			snd_seq_close(seq);
			fprintf(stderr, _("Connection failed (%s)\n"), snd_strerror(errno));
			return 1;
		}
	}

	snd_seq_close(seq);

	return 0;
}
