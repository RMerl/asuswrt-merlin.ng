/*
 * libwebsockets-test-fraggle - random fragmentation test
 *
 * Copyright (C) 2010-2011 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "../lib/libwebsockets.h"

#define LOCAL_RESOURCE_PATH INSTALL_DATADIR"/libwebsockets-test-server"

static int client;
static int terminate;

enum demo_protocols {
	PROTOCOL_FRAGGLE,

	/* always last */
	DEMO_PROTOCOL_COUNT
};

/* fraggle protocol */

enum fraggle_states {
	FRAGSTATE_START_MESSAGE,
	FRAGSTATE_RANDOM_PAYLOAD,
	FRAGSTATE_POST_PAYLOAD_SUM,
};

struct per_session_data__fraggle {
	int packets_left;
	int total_message;
	unsigned long sum;
	enum fraggle_states state;
};

static int
callback_fraggle(struct lws *wsi, enum lws_callback_reasons reason,
		 void *user, void *in, size_t len)
{
	int n;
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 8000 +
			  LWS_SEND_BUFFER_POST_PADDING];
	struct per_session_data__fraggle *psf = user;
	int chunk;
	int write_mode = LWS_WRITE_CONTINUATION;
	unsigned long sum;
	unsigned char *p = (unsigned char *)in;
	unsigned char *bp = &buf[LWS_SEND_BUFFER_PRE_PADDING];
	int ran;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:

		fprintf(stderr, "server sees client connect\n");
		psf->state = FRAGSTATE_START_MESSAGE;
		/* start the ball rolling */
		lws_callback_on_writable(wsi);
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:

		fprintf(stderr, "client connects to server\n");
		psf->state = FRAGSTATE_START_MESSAGE;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:

		switch (psf->state) {

		case FRAGSTATE_START_MESSAGE:

			psf->state = FRAGSTATE_RANDOM_PAYLOAD;
			psf->sum = 0;
			psf->total_message = 0;
			psf->packets_left = 0;

			/* fallthru */

		case FRAGSTATE_RANDOM_PAYLOAD:

			for (n = 0; n < len; n++)
				psf->sum += p[n];

			psf->total_message += len;
			psf->packets_left++;

			if (lws_is_final_fragment(wsi))
				psf->state = FRAGSTATE_POST_PAYLOAD_SUM;
			break;

		case FRAGSTATE_POST_PAYLOAD_SUM:

			sum = ((unsigned int)p[0]) << 24;
			sum |= p[1] << 16;
			sum |= p[2] << 8;
			sum |= p[3];
			if (sum == psf->sum)
				fprintf(stderr, "EOM received %d correctly "
						"from %d fragments\n",
					psf->total_message, psf->packets_left);
			else
				fprintf(stderr, "**** ERROR at EOM: "
						"length %d, rx sum = 0x%lX, "
						"server says it sent 0x%lX\n",
					     psf->total_message, psf->sum, sum);

			psf->state = FRAGSTATE_START_MESSAGE;
			break;
		}
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:

		switch (psf->state) {

		case FRAGSTATE_START_MESSAGE:
			lws_get_random(lws_get_context(wsi), &ran, sizeof(ran));
			psf->packets_left = (ran % 1024) + 1;
			fprintf(stderr, "Spamming %d random fragments\n",
							     psf->packets_left);
			psf->sum = 0;
			psf->total_message = 0;
			write_mode = LWS_WRITE_BINARY;
			psf->state = FRAGSTATE_RANDOM_PAYLOAD;

			/* fallthru */

		case FRAGSTATE_RANDOM_PAYLOAD:

			/*
			 * note how one chunk can be 8000, but we use the
			 * default rx buffer size of 4096, so we exercise the
			 * code for rx spill because the rx buffer is full
			 */

			lws_get_random(lws_get_context(wsi), &ran, sizeof(ran));
			chunk = (ran % 8000) + 1;
			psf->total_message += chunk;

			lws_get_random(lws_get_context(wsi), bp, chunk);
			for (n = 0; n < chunk; n++)
				psf->sum += bp[n];

			psf->packets_left--;
			if (psf->packets_left)
				write_mode |= LWS_WRITE_NO_FIN;
			else
				psf->state = FRAGSTATE_POST_PAYLOAD_SUM;

			n = lws_write(wsi, bp, chunk, write_mode);
			if (n < 0)
				return -1;
			if (n < chunk) {
				lwsl_err("Partial write\n");
				return -1;
			}

			lws_callback_on_writable(wsi);
			break;

		case FRAGSTATE_POST_PAYLOAD_SUM:

			fprintf(stderr, "Spamming session over, "
					"len = %d. sum = 0x%lX\n",
						  psf->total_message, psf->sum);

			bp[0] = psf->sum >> 24;
			bp[1] = psf->sum >> 16;
			bp[2] = psf->sum >> 8;
			bp[3] = psf->sum;

			n = lws_write(wsi, (unsigned char *)bp,
							   4, LWS_WRITE_BINARY);
			if (n < 0)
				return -1;
			if (n < 4) {
				lwsl_err("Partial write\n");
				return -1;
			}

			psf->state = FRAGSTATE_START_MESSAGE;

			lws_callback_on_writable(wsi);
			break;
		}
		break;

	case LWS_CALLBACK_CLOSED:

		terminate = 1;
		break;

	/* because we are protocols[0] ... */

	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
		if (strcmp(in, "deflate-stream") == 0) {
			fprintf(stderr, "denied deflate-stream extension\n");
			return 1;
		}
		break;

	default:
		break;
	}

	return 0;
}


/* list of supported protocols and callbacks */

static struct lws_protocols protocols[] = {
	{
		"fraggle-protocol",
		callback_fraggle,
		sizeof(struct per_session_data__fraggle),
	},
	{
		NULL, NULL, 0		/* End of list */
	}
};

static struct option options[] = {
	{ "help",	no_argument,		NULL, 'h' },
	{ "debug",	required_argument,	NULL, 'd' },
	{ "port",	required_argument,	NULL, 'p' },
	{ "ssl",	no_argument,		NULL, 's' },
	{ "interface",  required_argument,	NULL, 'i' },
	{ "client",	no_argument,		NULL, 'c' },
	{ NULL, 0, 0, 0 }
};

int main(int argc, char **argv)
{
	int n = 0;
	int port = 7681;
	int use_ssl = 0;
	struct lws_context *context;
	int opts = 0;
	char interface_name[128] = "";
	const char *iface = NULL;
	struct lws *wsi;
	const char *address;
	int server_port = port;
	struct lws_context_creation_info info;

	memset(&info, 0, sizeof info);

	fprintf(stderr, "libwebsockets test fraggle\n"
			"(C) Copyright 2010-2015 Andy Green <andy@warmcat.com> "
						    "licensed under LGPL2.1\n");

	while (n >= 0) {
		n = getopt_long(argc, argv, "ci:hsp:d:", options, NULL);
		if (n < 0)
			continue;
		switch (n) {
		case 'd':
			lws_set_log_level(atoi(optarg), NULL);
			break;
		case 's':
			use_ssl = 1;
			break;
		case 'p':
			port = atoi(optarg);
			server_port = port;
			break;
		case 'i':
			strncpy(interface_name, optarg, sizeof interface_name);
			interface_name[(sizeof interface_name) - 1] = '\0';
			iface = interface_name;
			break;
		case 'c':
			client = 1;
			fprintf(stderr, " Client mode\n");
			break;
		case 'h':
			fprintf(stderr, "Usage: libwebsockets-test-fraggle "
					"[--port=<p>] [--ssl] "
					"[-d <log bitfield>] "
					"[--client]\n");
			exit(1);
		}
	}

	if (client) {
		server_port = CONTEXT_PORT_NO_LISTEN;
		if (optind >= argc) {
			fprintf(stderr, "Must give address of server\n");
			return 1;
		}
	}

	info.port = server_port;
	info.iface = iface;
	info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
	info.extensions = lws_get_internal_extensions();
#endif
	if (use_ssl) {
		info.ssl_cert_filepath = LOCAL_RESOURCE_PATH"/libwebsockets-test-server.pem";
		info.ssl_private_key_filepath = LOCAL_RESOURCE_PATH"/libwebsockets-test-server.key.pem";
	}
	info.gid = -1;
	info.uid = -1;
	info.options = opts;

	context = lws_create_context(&info);
	if (context == NULL) {
		fprintf(stderr, "libwebsocket init failed\n");
		return -1;
	}

	if (client) {
		address = argv[optind];
		fprintf(stderr, "Connecting to %s:%u\n", address, port);
		wsi = lws_client_connect(context, address,
						   port, use_ssl, "/", address,
				 "origin", protocols[PROTOCOL_FRAGGLE].name,
								  -1);
		if (wsi == NULL) {
			fprintf(stderr, "Client connect to server failed\n");
			goto bail;
		}
	}

	n = 0;
	while (!n && !terminate)
		n = lws_service(context, 50);

	fprintf(stderr, "Terminating...\n");

bail:
	lws_context_destroy(context);

	return 0;
}
