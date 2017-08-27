/*
 * libwebsockets-test-client - libwebsockets test implementation
 *
 * Copyright (C) 2011 Andy Green <andy@warmcat.com>
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
#include <signal.h>

#ifdef _WIN32
#define random rand
#include "gettimeofday.h"
#else
#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include "../lib/libwebsockets.h"

static int deny_deflate, deny_mux, longlived, mirror_lifetime;
static struct lws *wsi_dumb, *wsi_mirror;
static volatile int force_exit;
static unsigned int opts;

/*
 * This demo shows how to connect multiple websockets simultaneously to a
 * websocket server (there is no restriction on their having to be the same
 * server just it simplifies the demo).
 *
 *  dumb-increment-protocol:  we connect to the server and print the number
 *				we are given
 *
 *  lws-mirror-protocol: draws random circles, which are mirrored on to every
 *				client (see them being drawn in every browser
 *				session also using the test server)
 */

enum demo_protocols {

	PROTOCOL_DUMB_INCREMENT,
	PROTOCOL_LWS_MIRROR,

	/* always last */
	DEMO_PROTOCOL_COUNT
};


/*
 * dumb_increment protocol
 *
 * since this also happens to be protocols[0], some callbacks that are not
 * bound to a specific protocol also turn up here.
 */

static int
callback_dumb_increment(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len)
{
	switch (reason) {

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lwsl_info("dumb: LWS_CALLBACK_CLIENT_ESTABLISHED\n");
		break;

	case LWS_CALLBACK_CLOSED:
		lwsl_notice("dumb: LWS_CALLBACK_CLOSED\n");
		wsi_dumb = NULL;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		((char *)in)[len] = '\0';
		lwsl_info("rx %d '%s'\n", (int)len, (char *)in);
		break;

	/* because we are protocols[0] ... */

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		if (wsi == wsi_dumb) {
			lwsl_err("dumb: LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
			wsi_dumb = NULL;
		}
		if (wsi == wsi_mirror) {
			lwsl_err("mirror: LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
			wsi_mirror = NULL;
		}
		break;

	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
		if ((strcmp(in, "deflate-stream") == 0) && deny_deflate) {
			lwsl_notice("denied deflate-stream extension\n");
			return 1;
		}
		if ((strcmp(in, "deflate-frame") == 0) && deny_deflate) {
			lwsl_notice("denied deflate-frame extension\n");
			return 1;
		}
		if ((strcmp(in, "x-google-mux") == 0) && deny_mux) {
			lwsl_notice("denied x-google-mux extension\n");
			return 1;
		}
		break;

	default:
		break;
	}

	return 0;
}


/* lws-mirror_protocol */


static int
callback_lws_mirror(struct lws *wsi, enum lws_callback_reasons reason,
		    void *user, void *in, size_t len)
{
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 4096 +
			  LWS_SEND_BUFFER_POST_PADDING];
	unsigned int rands[4];
	int l = 0;
	int n;

	switch (reason) {
	case LWS_CALLBACK_CLIENT_ESTABLISHED:

		lwsl_notice("mirror: LWS_CALLBACK_CLIENT_ESTABLISHED\n");

		lws_get_random(lws_get_context(wsi), rands, sizeof(rands[0]));
		mirror_lifetime = 16384 + (rands[0] & 65535);
		/* useful to test single connection stability */
		if (longlived)
			mirror_lifetime += 500000;

		lwsl_info("opened mirror connection with "
			  "%d lifetime\n", mirror_lifetime);

		/*
		 * mirror_lifetime is decremented each send, when it reaches
		 * zero the connection is closed in the send callback.
		 * When the close callback comes, wsi_mirror is set to NULL
		 * so a new connection will be opened
		 *
		 * start the ball rolling,
		 * LWS_CALLBACK_CLIENT_WRITEABLE will come next service
		 */
		lws_callback_on_writable(wsi);
		break;

	case LWS_CALLBACK_CLOSED:
		lwsl_notice("mirror: LWS_CALLBACK_CLOSED mirror_lifetime=%d\n", mirror_lifetime);
		wsi_mirror = NULL;
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		for (n = 0; n < 1; n++) {
			lws_get_random(lws_get_context(wsi), rands, sizeof(rands));
			l += sprintf((char *)&buf[LWS_SEND_BUFFER_PRE_PADDING + l],
					"c #%06X %d %d %d;",
					(int)rands[0] & 0xffffff,
					(int)rands[1] % 500,
					(int)rands[2] % 250,
					(int)rands[3] % 24);
		}

		n = lws_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], l,
			      opts | LWS_WRITE_TEXT);
		if (n < 0)
			return -1;
		if (n < l) {
			lwsl_err("Partial write LWS_CALLBACK_CLIENT_WRITEABLE\n");
			return -1;
		}

		mirror_lifetime--;
		if (!mirror_lifetime) {
			lwsl_info("closing mirror session\n");
			return -1;
		}
		/* get notified as soon as we can write again */
		lws_callback_on_writable(wsi);
		break;

	default:
		break;
	}

	return 0;
}


/* list of supported protocols and callbacks */

static struct lws_protocols protocols[] = {
	{
		"dumb-increment-protocol,fake-nonexistant-protocol",
		callback_dumb_increment,
		0,
		20,
	},
	{
		"fake-nonexistant-protocol,lws-mirror-protocol",
		callback_lws_mirror,
		0,
		128,
	},
	{ NULL, NULL, 0, 0 } /* end */
};

void sighandler(int sig)
{
	force_exit = 1;
}

static struct option options[] = {
	{ "help",	no_argument,		NULL, 'h' },
	{ "debug",      required_argument,      NULL, 'd' },
	{ "port",	required_argument,	NULL, 'p' },
	{ "ssl",	no_argument,		NULL, 's' },
	{ "version",	required_argument,	NULL, 'v' },
	{ "undeflated",	no_argument,		NULL, 'u' },
	{ "nomux",	no_argument,		NULL, 'n' },
	{ "longlived",	no_argument,		NULL, 'l' },
	{ NULL, 0, 0, 0 }
};

static int ratelimit_connects(unsigned int *last, int secs)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	if (tv.tv_sec - (*last) < secs)
		return 0;

	*last = tv.tv_sec;

	return 1;
}

int main(int argc, char **argv)
{
	int n = 0, ret = 0, port = 7681, use_ssl = 0;
	unsigned int rl_dumb = 0, rl_mirror = 0;
	struct lws_context_creation_info info;
	struct lws_context *context;
	int ietf_version = -1; /* latest */
	const char *address;

	memset(&info, 0, sizeof info);

	fprintf(stderr, "libwebsockets test client\n"
			"(C) Copyright 2010-2015 Andy Green <andy@warmcat.com> "
			"licensed under LGPL2.1\n");

	if (argc < 2)
		goto usage;

	while (n >= 0) {
		n = getopt_long(argc, argv, "nuv:hsp:d:l", options, NULL);
		if (n < 0)
			continue;
		switch (n) {
		case 'd':
			lws_set_log_level(atoi(optarg), NULL);
			break;
		case 's':
			use_ssl = 2; /* 2 = allow selfsigned */
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'l':
			longlived = 1;
			break;
		case 'v':
			ietf_version = atoi(optarg);
			break;
		case 'u':
			deny_deflate = 1;
			break;
		case 'n':
			deny_mux = 1;
			break;
		case 'h':
			goto usage;
		}
	}

	if (optind >= argc)
		goto usage;

	signal(SIGINT, sighandler);

	address = argv[optind];

	/*
	 * create the websockets context.  This tracks open connections and
	 * knows how to route any traffic and which protocol version to use,
	 * and if each connection is client or server side.
	 *
	 * For this client-only demo, we tell it to not listen on any port.
	 */

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
	info.extensions = lws_get_internal_extensions();
#endif
	info.gid = -1;
	info.uid = -1;

	context = lws_create_context(&info);
	if (context == NULL) {
		fprintf(stderr, "Creating libwebsocket context failed\n");
		return 1;
	}

	/*
	 * sit there servicing the websocket context to handle incoming
	 * packets, and drawing random circles on the mirror protocol websocket
	 *
	 * nothing happens until the client websocket connection is
	 * asynchronously established... calling lws_client_connect() only
	 * instantiates the connection logically, lws_service() progresses it
	 * asynchronously.
	 */

	while (!force_exit) {

		if (!wsi_dumb && ratelimit_connects(&rl_dumb, 2)) {
			lwsl_notice("dumb: connecting\n");
			wsi_dumb = lws_client_connect(context, address, port,
				use_ssl, "/", argv[optind], argv[optind],
				protocols[PROTOCOL_DUMB_INCREMENT].name,
				ietf_version);
		}

		if (!wsi_mirror && ratelimit_connects(&rl_mirror, 2)) {
			lwsl_notice("mirror: connecting\n");
			wsi_mirror = lws_client_connect(context,
				address, port, use_ssl,  "/",
				argv[optind], argv[optind],
				protocols[PROTOCOL_LWS_MIRROR].name,
				ietf_version);
		}

		lws_service(context, 500);
	}

	lwsl_err("Exiting\n");
	lws_context_destroy(context);

	return ret;

usage:
	fprintf(stderr, "Usage: libwebsockets-test-client "
				"<server address> [--port=<p>] "
				"[--ssl] [-k] [-v <ver>] "
				"[-d <log bitfield>] [-l]\n");
	return 1;
}
