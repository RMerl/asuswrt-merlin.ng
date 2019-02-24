/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
#include <sys/socket.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include <library.h>
#include <utils/debug.h>
#include <tls_socket.h>
#include <networking/host.h>
#include <credentials/sets/mem_cred.h>

/**
 * Print usage information
 */
static void usage(FILE *out, char *cmd)
{
	fprintf(out, "usage:\n");
	fprintf(out, "  %s --connect <address> --port <port> [--key <key] [--cert <file>]+ [--times <n>]\n", cmd);
	fprintf(out, "  %s --listen <address> --port <port> --key <key> [--cert <file>]+ [--times <n>]\n", cmd);
}

/**
 * Check, as client, if we have a client certificate with private key
 */
static identification_t *find_client_id()
{
	identification_t *client = NULL, *keyid;
	enumerator_t *enumerator;
	certificate_t *cert;
	public_key_t *pubkey;
	private_key_t *privkey;
	chunk_t chunk;

	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
											CERT_X509, KEY_ANY, NULL, FALSE);
	while (enumerator->enumerate(enumerator, &cert))
	{
		pubkey = cert->get_public_key(cert);
		if (pubkey)
		{
			if (pubkey->get_fingerprint(pubkey, KEYID_PUBKEY_SHA1, &chunk))
			{
				keyid = identification_create_from_encoding(ID_KEY_ID, chunk);
				privkey = lib->credmgr->get_private(lib->credmgr,
									pubkey->get_type(pubkey), keyid, NULL);
				keyid->destroy(keyid);
				if (privkey)
				{
					client = cert->get_subject(cert);
					client = client->clone(client);
					privkey->destroy(privkey);
				}
			}
			pubkey->destroy(pubkey);
		}
		if (client)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	return client;
}

/**
 * Client routine
 */
static int run_client(host_t *host, identification_t *server,
					  identification_t *client, int times, tls_cache_t *cache)
{
	tls_socket_t *tls;
	int fd, res;

	while (times == -1 || times-- > 0)
	{
		fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd == -1)
		{
			DBG1(DBG_TLS, "opening socket failed: %s", strerror(errno));
			return 1;
		}
		if (connect(fd, host->get_sockaddr(host),
					*host->get_sockaddr_len(host)) == -1)
		{
			DBG1(DBG_TLS, "connecting to %#H failed: %s", host, strerror(errno));
			close(fd);
			return 1;
		}
		tls = tls_socket_create(FALSE, server, client, fd, cache, TLS_1_2, TRUE);
		if (!tls)
		{
			close(fd);
			return 1;
		}
		res = tls->splice(tls, 0, 1) ? 0 : 1;
		tls->destroy(tls);
		close(fd);
		if (res)
		{
			break;
		}
	}
	return res;
}

/**
 * Server routine
 */
static int serve(host_t *host, identification_t *server,
				 int times, tls_cache_t *cache)
{
	tls_socket_t *tls;
	int fd, cfd;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		DBG1(DBG_TLS, "opening socket failed: %s", strerror(errno));
		return 1;
	}
	if (bind(fd, host->get_sockaddr(host),
			 *host->get_sockaddr_len(host)) == -1)
	{
		DBG1(DBG_TLS, "binding to %#H failed: %s", host, strerror(errno));
		close(fd);
		return 1;
	}
	if (listen(fd, 1) == -1)
	{
		DBG1(DBG_TLS, "listen to %#H failed: %m", host, strerror(errno));
		close(fd);
		return 1;
	}

	while (times == -1 || times-- > 0)
	{
		cfd = accept(fd, host->get_sockaddr(host), host->get_sockaddr_len(host));
		if (cfd == -1)
		{
			DBG1(DBG_TLS, "accept failed: %s", strerror(errno));
			close(fd);
			return 1;
		}
		DBG1(DBG_TLS, "%#H connected", host);

		tls = tls_socket_create(TRUE, server, NULL, cfd, cache, TLS_1_2, TRUE);
		if (!tls)
		{
			close(fd);
			return 1;
		}
		tls->splice(tls, 0, 1);
		DBG1(DBG_TLS, "%#H disconnected", host);
		tls->destroy(tls);
	}
	close(fd);

	return 0;
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
 * TLS debug level
 */
static level_t tls_level = 1;

static void dbg_tls(debug_t group, level_t level, char *fmt, ...)
{
	if ((group == DBG_TLS && level <= tls_level) || level <= 1)
	{
		va_list args;

		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		va_end(args);
	}
}

/**
 * Cleanup
 */
static void cleanup()
{
	lib->credmgr->remove_set(lib->credmgr, &creds->set);
	creds->destroy(creds);
	library_deinit();
}

/**
 * Initialize library
 */
static void init()
{
	library_init(NULL, "tls_test");

	dbg = dbg_tls;

	lib->plugins->load(lib->plugins, PLUGINS);

	creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &creds->set);

	atexit(cleanup);
}

int main(int argc, char *argv[])
{
	char *address = NULL;
	bool listen = FALSE;
	int port = 0, times = -1, res;
	identification_t *server, *client;
	tls_cache_t *cache;
	host_t *host;

	init();

	while (TRUE)
	{
		struct option long_opts[] = {
			{"help",		no_argument,			NULL,		'h' },
			{"connect",		required_argument,		NULL,		'c' },
			{"listen",		required_argument,		NULL,		'l' },
			{"port",		required_argument,		NULL,		'p' },
			{"cert",		required_argument,		NULL,		'x' },
			{"key",			required_argument,		NULL,		'k' },
			{"times",		required_argument,		NULL,		't' },
			{"debug",		required_argument,		NULL,		'd' },
			{0,0,0,0 }
		};
		switch (getopt_long(argc, argv, "", long_opts, NULL))
		{
			case EOF:
				break;
			case 'h':
				usage(stdout, argv[0]);
				return 0;
			case 'x':
				if (!load_certificate(optarg))
				{
					return 1;
				}
				continue;
			case 'k':
				if (!load_key(optarg))
				{
					return 1;
				}
				continue;
			case 'l':
				listen = TRUE;
				/* fall */
			case 'c':
				if (address)
				{
					usage(stderr, argv[0]);
					return 1;
				}
				address = optarg;
				continue;
			case 'p':
				port = atoi(optarg);
				continue;
			case 't':
				times = atoi(optarg);
				continue;
			case 'd':
				tls_level = atoi(optarg);
				continue;
			default:
				usage(stderr, argv[0]);
				return 1;
		}
		break;
	}
	if (!port || !address)
	{
		usage(stderr, argv[0]);
		return 1;
	}
	host = host_create_from_dns(address, 0, port);
	if (!host)
	{
		DBG1(DBG_TLS, "resolving hostname %s failed", address);
		return 1;
	}
	server = identification_create_from_string(address);
	cache = tls_cache_create(100, 30);
	if (listen)
	{
		res = serve(host, server, times, cache);
	}
	else
	{
		client = find_client_id();
		res = run_client(host, server, client, times, cache);
		DESTROY_IF(client);
	}
	cache->destroy(cache);
	host->destroy(host);
	server->destroy(server);
	return res;
}
