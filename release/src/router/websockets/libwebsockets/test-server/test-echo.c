/*
 * libwebsockets-test-echo - libwebsockets echo test implementation
 *
 * This implements both the client and server sides.  It defaults to
 * serving, use --client <remote address> to connect as client.
 *
 * Copyright (C) 2010-2013 Andy Green <andy@warmcat.com>
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
#include <assert.h>
#include <signal.h>

#include "../lib/libwebsockets.h"

#ifndef _WIN32
#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>
#else
#include "gettimeofday.h"
#include <process.h>
#endif

static volatile int force_exit = 0;
static int versa, state;

#define MAX_ECHO_PAYLOAD 1400
#define LOCAL_RESOURCE_PATH INSTALL_DATADIR"/libwebsockets-test-server"

struct per_session_data__echo {
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + MAX_ECHO_PAYLOAD + LWS_SEND_BUFFER_POST_PADDING];
	unsigned int len;
	unsigned int index;
};

static int
callback_echo(struct lws *wsi, enum lws_callback_reasons reason, void *user,
	      void *in, size_t len)
{
	struct per_session_data__echo *pss = (struct per_session_data__echo *)user;
	int n;

	switch (reason) {

#ifndef LWS_NO_SERVER
	/* when the callback is used for server operations --> */

	case LWS_CALLBACK_SERVER_WRITEABLE:
do_tx:
		n = lws_write(wsi, &pss->buf[LWS_SEND_BUFFER_PRE_PADDING], pss->len, LWS_WRITE_TEXT);
		if (n < 0) {
			lwsl_err("ERROR %d writing to socket, hanging up\n", n);
			return 1;
		}
		if (n < (int)pss->len) {
			lwsl_err("Partial write\n");
			return -1;
		}
		break;

	case LWS_CALLBACK_RECEIVE:
do_rx:
		if (len > MAX_ECHO_PAYLOAD) {
			lwsl_err("Server received packet bigger than %u, hanging up\n", MAX_ECHO_PAYLOAD);
			return 1;
		}
		memcpy(&pss->buf[LWS_SEND_BUFFER_PRE_PADDING], in, len);
		pss->len = (unsigned int)len;
		lws_callback_on_writable(wsi);
		break;
#endif

#ifndef LWS_NO_CLIENT
	/* when the callback is used for client operations --> */

	case LWS_CALLBACK_CLOSED:
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_info("closed\n");
		state = 0;
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lwsl_notice("Client has connected\n");
		pss->index = 0;
		state = 2;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
#ifndef LWS_NO_SERVER
		if (versa)
			goto do_rx;
#endif
		lwsl_notice("Client RX: %s", (char *)in);
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
#ifndef LWS_NO_SERVER
		if (versa)
			goto do_tx;
#endif
		/* we will send our packet... */
		pss->len = sprintf((char *)&pss->buf[LWS_SEND_BUFFER_PRE_PADDING], "hello from libwebsockets-test-echo client pid %d index %d\n", getpid(), pss->index++);
		lwsl_notice("Client TX: %s", &pss->buf[LWS_SEND_BUFFER_PRE_PADDING]);
		n = lws_write(wsi, &pss->buf[LWS_SEND_BUFFER_PRE_PADDING], pss->len, LWS_WRITE_TEXT);
		if (n < 0) {
			lwsl_err("ERROR %d writing to socket, hanging up\n", n);
			return -1;
		}
		if (n < (int)pss->len) {
			lwsl_err("Partial write\n");
			return -1;
		}
		break;
	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:

		break;
#endif
	default:
		break;
	}

	return 0;
}



static struct lws_protocols protocols[] = {
	/* first protocol must always be HTTP handler */

	{
		"default",		/* name */
		callback_echo,		/* callback */
		sizeof(struct per_session_data__echo)	/* per_session_data_size */
	},
	{
		NULL, NULL, 0		/* End of list */
	}
};

void sighandler(int sig)
{
	force_exit = 1;
}

static struct option options[] = {
	{ "help",	no_argument,		NULL, 'h' },
	{ "debug",	required_argument,	NULL, 'd' },
	{ "port",	required_argument,	NULL, 'p' },
	{ "ssl-cert",	required_argument, 	NULL, 'C' },
	{ "ssl-key",	required_argument,	NULL, 'k' },
#ifndef LWS_NO_CLIENT
	{ "client",	required_argument,	NULL, 'c' },
	{ "ratems",	required_argument,	NULL, 'r' },
#endif
	{ "ssl",	no_argument,		NULL, 's' },
	{ "versa",	no_argument,		NULL, 'v' },
	{ "uri",	required_argument,	NULL, 'u' },
	{ "passphrase", required_argument,	NULL, 'P' },
	{ "interface",  required_argument,	NULL, 'i' },
#ifndef LWS_NO_DAEMONIZE
	{ "daemonize", 	no_argument,		NULL, 'D' },
#endif
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
	const char *_interface = NULL;
	char ssl_cert[256] = LOCAL_RESOURCE_PATH"/libwebsockets-test-server.pem";
	char ssl_key[256] = LOCAL_RESOURCE_PATH"/libwebsockets-test-server.key.pem";
#ifndef _WIN32
	int syslog_options = LOG_PID | LOG_PERROR;
#endif
	int client = 0;
	int listen_port = 80;
	struct lws_context_creation_info info;
	char passphrase[256];
	char uri[256] = "/";
#ifndef LWS_NO_CLIENT
	char address[256], ads_port[256 + 30];
	int rate_us = 250000;
	unsigned long long oldus;
	struct lws *wsi;
	int disallow_selfsigned = 0;
	struct timeval tv;
#endif

	int debug_level = 7;
#ifndef LWS_NO_DAEMONIZE
	int daemonize = 0;
#endif

	memset(&info, 0, sizeof info);

#ifndef LWS_NO_CLIENT
	lwsl_notice("Built to support client operations\n");
#endif
#ifndef LWS_NO_SERVER
	lwsl_notice("Built to support server operations\n");
#endif

	while (n >= 0) {
		n = getopt_long(argc, argv, "i:hsp:d:DC:k:P:vu:"
#ifndef LWS_NO_CLIENT
			"c:r:"
#endif
				, options, NULL);
		if (n < 0)
			continue;
		switch (n) {
		case 'P':
			strncpy(passphrase, optarg, sizeof(passphrase));
			passphrase[sizeof(passphrase) - 1] = '\0';
			info.ssl_private_key_password = passphrase;
			break;
		case 'C':
			strncpy(ssl_cert, optarg, sizeof(ssl_cert));
			ssl_cert[sizeof(ssl_cert) - 1] = '\0';
			disallow_selfsigned = 1;
			break;
		case 'k':
			strncpy(ssl_key, optarg, sizeof(ssl_key));
			ssl_key[sizeof(ssl_key) - 1] = '\0';
			break;
		case 'u':
			strncpy(uri, optarg, sizeof(uri));
			uri[sizeof(uri) - 1] = '\0';
			break;

#ifndef LWS_NO_DAEMONIZE
		case 'D':
			daemonize = 1;
#ifndef _WIN32
			syslog_options &= ~LOG_PERROR;
#endif
			break;
#endif
#ifndef LWS_NO_CLIENT
		case 'c':
			client = 1;
			strncpy(address, optarg, sizeof(address) - 1);
			address[sizeof(address) - 1] = '\0';
			port = 80;
			break;
		case 'r':
			rate_us = atoi(optarg) * 1000;
			break;
#endif
		case 'd':
			debug_level = atoi(optarg);
			break;
		case 's':
			use_ssl = 1; /* 1 = take care about cert verification, 2 = allow anything */
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'v':
			versa = 1;
			break;
		case 'i':
			strncpy(interface_name, optarg, sizeof interface_name);
			interface_name[(sizeof interface_name) - 1] = '\0';
			_interface = interface_name;
			break;
		case '?':
		case 'h':
			fprintf(stderr, "Usage: libwebsockets-test-echo\n"
				"  --debug      / -d <debug bitfield>\n"
				"  --port       / -p <port>\n"
				"  --ssl-cert   / -C <cert path>\n"
				"  --ssl-key    / -k <key path>\n"
#ifndef LWS_NO_CLIENT
				"  --client     / -c <server IP>\n"
				"  --ratems     / -r <rate in ms>\n"
#endif
				"  --ssl        / -s\n"
				"  --passphrase / -P <passphrase>\n"
				"  --interface  / -i <interface>\n"
#ifndef LWS_NO_DAEMONIZE
				"  --daemonize  / -D\n"
#endif
			);
			exit(1);
		}
	}

#ifndef LWS_NO_DAEMONIZE
	/*
	 * normally lock path would be /var/lock/lwsts or similar, to
	 * simplify getting started without having to take care about
	 * permissions or running as root, set to /tmp/.lwsts-lock
	 */
#if defined(WIN32) || defined(_WIN32)
#else
	if (!client && daemonize && lws_daemonize("/tmp/.lwstecho-lock")) {
		fprintf(stderr, "Failed to daemonize\n");
		return 1;
	}
#endif
#endif

#ifndef _WIN32
	/* we will only try to log things according to our debug_level */
	setlogmask(LOG_UPTO (LOG_DEBUG));
	openlog("lwsts", syslog_options, LOG_DAEMON);
#endif

	/* tell the library what debug level to emit and to send it to syslog */
	lws_set_log_level(debug_level, lwsl_emit_syslog);

	lwsl_notice("libwebsockets echo test - "
		    "(C) Copyright 2010-2015 Andy Green <andy@warmcat.com> - "
		    "licensed under LGPL2.1\n");
#ifndef LWS_NO_CLIENT
	if (client) {
		lwsl_notice("Running in client mode\n");
		listen_port = CONTEXT_PORT_NO_LISTEN;
		if (use_ssl && !disallow_selfsigned) {
			lwsl_info("allowing selfsigned\n");
			use_ssl = 2;
		} else {
			lwsl_info("requiring server cert validation againts %s\n", ssl_cert);
			info.ssl_ca_filepath = ssl_cert;
		}
	} else {
#endif
#ifndef LWS_NO_SERVER
		lwsl_notice("Running in server mode\n");
		listen_port = port;
#endif
#ifndef LWS_NO_CLIENT
	}
#endif

	info.port = listen_port;
	info.iface = _interface;
	info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
	info.extensions = lws_get_internal_extensions();
#endif
	if (use_ssl && !client) {
		info.ssl_cert_filepath = ssl_cert;
		info.ssl_private_key_filepath = ssl_key;
	} else
		if (use_ssl && client) {
			info.ssl_cert_filepath = NULL;
			info.ssl_private_key_filepath = NULL;
		}
	info.gid = -1;
	info.uid = -1;
	info.options = opts;

	context = lws_create_context(&info);
	if (context == NULL) {
		lwsl_err("libwebsocket init failed\n");
		return -1;
	}


	signal(SIGINT, sighandler);

#ifndef LWS_NO_CLIENT
	gettimeofday(&tv, NULL);
	oldus = ((unsigned long long)tv.tv_sec * 1000000) + tv.tv_usec;
#endif

	n = 0;
	while (n >= 0 && !force_exit) {
#ifndef LWS_NO_CLIENT
		if (client && !state) {
			state = 1;
			lwsl_notice("Client connecting to %s:%u....\n", address, port);
			/* we are in client mode */

			address[sizeof(address) - 1] = '\0';
			sprintf(ads_port, "%s:%u", address, port & 65535);

			wsi = lws_client_connect(context, address,
				port, use_ssl, uri, ads_port,
				 ads_port, NULL, -1);
			if (!wsi) {
				lwsl_err("Client failed to connect to %s:%u\n", address, port);
				goto bail;
			}
		}

		if (client && !versa) {
			gettimeofday(&tv, NULL);

			if (((((unsigned long long)tv.tv_sec * 1000000) + tv.tv_usec) - oldus) > rate_us) {
				lws_callback_on_writable_all_protocol(context,
						&protocols[0]);
				oldus = ((unsigned long long)tv.tv_sec * 1000000) + tv.tv_usec;
			}
		}
#endif
		n = lws_service(context, 10);
	}
#ifndef LWS_NO_CLIENT
bail:
#endif
	lws_context_destroy(context);

	lwsl_notice("libwebsockets-test-echo exited cleanly\n");
#ifndef _WIN32
	closelog();
#endif

	return 0;
}
