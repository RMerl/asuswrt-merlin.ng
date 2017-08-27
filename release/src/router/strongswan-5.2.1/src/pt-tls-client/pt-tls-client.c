/*
 * Copyright (C) 2010-2013 Martin Willi, revosec AG
 * Copyright (C) 2013-2014 Andreas Steffen
 * HSR Hochschule für Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_SYSLOG
#include <syslog.h>
#endif

#include <pt_tls.h>
#include <pt_tls_client.h>
#include <tnc/tnc.h>
#include <tls.h>

#include <library.h>
#include <utils/debug.h>
#include <credentials/sets/mem_cred.h>
#include <utils/optionsfrom.h>

/**
 * Print usage information
 */
static void usage(FILE *out)
{
	fprintf(out,
		"Usage: pt-tls  --connect <hostname|address> [--port <port>]\n"
		"              [--cert <file>]+ [--key <file>]\n"
		"              [--client <client-id>] [--secret <password>]\n"
		"              [--optionsfrom <filename>] [--quiet] [--debug <level>]\n");
}

/**
 * Client routine
 */
static int client(char *address, u_int16_t port, char *identity)
{
	pt_tls_client_t *assessment;
	tls_t *tnccs;
	identification_t *server, *client;
	host_t *host;
	status_t status;

	host = host_create_from_dns(address, AF_UNSPEC, port);
	if (!host)
	{
		return 1;
	}
	server = identification_create_from_string(address);
	client = identification_create_from_string(identity);
	tnccs = (tls_t*)tnc->tnccs->create_instance(tnc->tnccs, TNCCS_2_0, FALSE,
								server, client, TNC_IFT_TLS_2_0, NULL);
	if (!tnccs)
	{
		fprintf(stderr, "loading TNCCS failed: %s\n", PLUGINS);
		host->destroy(host);
		server->destroy(server);
		client->destroy(client);
		return 1;
	}
	assessment = pt_tls_client_create(host, server, client);
	status = assessment->run_assessment(assessment, (tnccs_t*)tnccs);
	assessment->destroy(assessment);
	tnccs->destroy(tnccs);
	return status;
}


/**
 * In-Memory credential set
 */
static mem_cred_t *creds;

/**
 * Load certificate from file
 */
static bool load_certificate(char *filename)
{
	certificate_t *cert;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_FROM_FILE, filename, BUILD_END);
	if (!cert)
	{
		DBG1(DBG_TLS, "loading certificate from '%s' failed", filename);
		return FALSE;
	}
	creds->add_cert(creds, TRUE, cert);
	return TRUE;
}

/**
 * Load private key from file
 */
static bool load_key(char *filename)
{
	private_key_t *key;

	key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
							 BUILD_FROM_FILE, filename, BUILD_END);
	if (!key)
	{
		DBG1(DBG_TLS, "loading key from '%s' failed", filename);
		return FALSE;
	}
	creds->add_key(creds, key);
	return TRUE;
}

/**
 * Logging and debug level
 */
static bool log_to_stderr = TRUE;
#ifdef HAVE_SYSLOG
static bool log_to_syslog = TRUE;
#endif /* HAVE_SYSLOG */
static level_t default_loglevel = 1;

static void dbg_pt_tls(debug_t group, level_t level, char *fmt, ...)
{
	va_list args;

	if (level <= default_loglevel)
	{
		if (log_to_stderr)
		{
			va_start(args, fmt);
			vfprintf(stderr, fmt, args);
			va_end(args);
			fprintf(stderr, "\n");
		}
#ifdef HAVE_SYSLOG
		if (log_to_syslog)
		{
			char buffer[8192];
			char *current = buffer, *next;

			/* write in memory buffer first */
			va_start(args, fmt);
			vsnprintf(buffer, sizeof(buffer), fmt, args);
			va_end(args);

			/* do a syslog with every line */
			while (current)
			{
				next = strchr(current, '\n');
				if (next)
				{
					*(next++) = '\0';
				}
				syslog(LOG_INFO, "%s\n", current);
				current = next;
			}
		}
#endif /* HAVE_SYSLOG */
	}
}

/**
 * Initialize logging to stderr/syslog
 */
static void init_log(const char *program)
{
	dbg = dbg_pt_tls;

	if (log_to_stderr)
	{
		setbuf(stderr, NULL);
	}
#ifdef HAVE_SYSLOG
	if (log_to_syslog)
	{
		openlog(program, LOG_CONS | LOG_NDELAY | LOG_PID, LOG_AUTHPRIV);
	}
#endif /* HAVE_SYSLOG */
}

/**
 * Handles --optionsfrom arguments
 */
options_t *options;

/**
 * Cleanup
 */
static void cleanup()
{
	lib->processor->cancel(lib->processor);
	lib->credmgr->remove_set(lib->credmgr, &creds->set);
	creds->destroy(creds);
	options->destroy(options);
	libtnccs_deinit();
	library_deinit();
}

/**
 * Initialize library
 */
static void init()
{
	plugin_feature_t features[] = {
		PLUGIN_NOOP,
			PLUGIN_PROVIDE(CUSTOM, "pt-tls-client"),
				PLUGIN_DEPENDS(CUSTOM, "tnccs-manager"),
	};
	library_init(NULL, "pt-tls-client");
	libtnccs_init();

	init_log("pt-tls-client");
	options = options_create();

	lib->plugins->add_static_features(lib->plugins, "pt-tls-client", features,
									  countof(features), TRUE, NULL, NULL);
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "pt-tls-client.load", PLUGINS)))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &creds->set);

	atexit(cleanup);
}

int main(int argc, char *argv[])
{
	char *address = NULL, *identity = "%any", *secret = NULL;
	int port = PT_TLS_PORT;

	init();

	while (TRUE)
	{
		struct option long_opts[] = {
			{"help",		no_argument,			NULL,		'h' },
			{"connect",		required_argument,		NULL,		'c' },
			{"client",		required_argument,		NULL,		'i' },
			{"secret",		required_argument,		NULL,		's' },
			{"port",		required_argument,		NULL,		'p' },
			{"cert",		required_argument,		NULL,		'x' },
			{"key",			required_argument,		NULL,		'k' },
			{"quiet",		no_argument,			NULL,		'q' },
			{"debug",		required_argument,		NULL,		'd' },
			{"optionsfrom",	required_argument,		NULL,		'+' },
			{0,0,0,0 }
		};
		switch (getopt_long(argc, argv, "", long_opts, NULL))
		{
			case EOF:
				break;
			case 'h':			/* --help */
				usage(stdout);
				return 0;
			case 'x':			/* --cert <file> */
				if (!load_certificate(optarg))
				{
					return 1;
				}
				continue;
			case 'k':			/* --key <file> */
				if (!load_key(optarg))
				{
					return 1;
				}
				continue;
			case 'c':			/* --connect <hostname|address> */
				if (address)
				{
					usage(stderr);
					return 1;
				}
				address = optarg;
				continue;
			case 'i':			/* --client <client-id> */
				identity = optarg;
				continue;
			case 's':			/* --secret <password> */
				secret = optarg;
				continue;
			case 'p':			/* --port <port> */
				port = atoi(optarg);
				continue;
			case 'q':       	/* --quiet */
				log_to_stderr = FALSE;
				continue;
			case 'd':			/* --debug <level> */
				default_loglevel = atoi(optarg);
				continue;
			case '+':			/* --optionsfrom <filename> */
				if (!options->from(options, optarg, &argc, &argv, optind))
				{
					return 1;
				}
				continue;
			default:
				usage(stderr);
				return 1;
		}
		break;
	}
	if (!address)
	{
		usage(stderr);
		return 1;
	}
	if (secret)
	{
		creds->add_shared(creds, shared_key_create(SHARED_EAP,
										chunk_clone(chunk_from_str(secret))),
							identification_create_from_string(identity), NULL);
	}

	return client(address, port, identity);
}
