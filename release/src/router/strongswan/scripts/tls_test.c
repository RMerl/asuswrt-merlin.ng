/*
 * Copyright (C) 2020 Pascal Knecht
 * Copyright (C) 2020 Tobias Brunner
 * Copyright (C) 2010 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
	fprintf(out, "  %s --connect <address> --port <port> [--key <key] [--cert <file>] [--cacert <file>]+ [--times <n>]\n", cmd);
	fprintf(out, "  %s --listen <address> --port <port> --key <key> --cert <file> [--cacert <file>]+ [--auth-optional] [--times <n>]\n", cmd);
	fprintf(out, "\n");
	fprintf(out, "options:\n");
	fprintf(out, "  --help                   print help and exit\n");
	fprintf(out, "  --connect <address>      connect to a server on dns name or ip address\n");
	fprintf(out, "  --listen <address>       listen on dns name or ip address\n");
	fprintf(out, "  --port <port>            specify the port to use\n");
	fprintf(out, "  --cert <file>            certificate to authenticate itself\n");
	fprintf(out, "  --key <file>             private key to authenticate itself\n");
	fprintf(out, "  --cacert <file>          certificate to verify other peer\n");
	fprintf(out, "  --identity <id>          optional remote identity to enforce\n");
	fprintf(out, "  --auth-optional          don't enforce client authentication\n");
	fprintf(out, "  --times <n>              specify the amount of repeated connection establishments\n");
	fprintf(out, "  --ipv4                   use IPv4\n");
	fprintf(out, "  --ipv6                   use IPv6\n");
	fprintf(out, "  --min-version <version>  specify the minimum TLS version, supported versions:\n");
	fprintf(out, "                           1.0 (default), 1.1, 1.2 and 1.3\n");
	fprintf(out, "  --max-version <version>  specify the maximum TLS version, supported versions:\n");
	fprintf(out, "                           1.0, 1.1, 1.2 and 1.3 (default)\n");
	fprintf(out, "  --version <version>      set one specific TLS version to use, supported versions:\n");
	fprintf(out, "                           1.0, 1.1, 1.2 and 1.3\n");
	fprintf(out, "  --debug <debug level>    set debug level, default is 1\n");
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
					  identification_t *client, int times, tls_cache_t *cache,
					  tls_version_t min_version, tls_version_t max_version,
					  tls_flag_t flags)
{
	tls_socket_t *tls;
	int fd, res;

	while (times == -1 || times-- > 0)
	{
		DBG2(DBG_TLS, "connecting to %#H", host);
		fd = socket(host->get_family(host), SOCK_STREAM, 0);
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
		tls = tls_socket_create(FALSE, server, client, fd, cache, min_version,
							    max_version, flags);
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
static int serve(host_t *host, identification_t *server, identification_t *client,
				 int times, tls_cache_t *cache, tls_version_t min_version,
				 tls_version_t max_version, tls_flag_t flags)
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

		tls = tls_socket_create(TRUE, server, client, cfd, cache, min_version,
								max_version, flags);
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

	key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_ANY,
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
	char *plugins;

	library_init(NULL, "tls_test");

	dbg = dbg_tls;

	plugins = getenv("PLUGINS") ?: PLUGINS;
	lib->plugins->load(lib->plugins, plugins);

	creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &creds->set);

	atexit(cleanup);
}

int main(int argc, char *argv[])
{
	char *address = NULL;
	bool listen = FALSE;
	int port = 0, times = -1, res, family = AF_UNSPEC;
	identification_t *server, *client = NULL, *identity = NULL;
	tls_version_t min_version = TLS_SUPPORTED_MIN, max_version = TLS_SUPPORTED_MAX;
	tls_flag_t flags = TLS_FLAG_ENCRYPTION_OPTIONAL;
	tls_cache_t *cache;
	host_t *host;

	init();

	while (TRUE)
	{
		struct option long_opts[] = {
			{"help",			no_argument,			NULL,		'h' },
			{"connect",			required_argument,		NULL,		'c' },
			{"listen",			required_argument,		NULL,		'l' },
			{"port",			required_argument,		NULL,		'p' },
			{"cert",			required_argument,		NULL,		'x' },
			{"key",				required_argument,		NULL,		'k' },
			{"cacert",			required_argument,		NULL,		'f' },
			{"times",			required_argument,		NULL,		't' },
			{"ipv4",			no_argument,			NULL,		'4' },
			{"ipv6",			no_argument,			NULL,		'6' },
			{"min-version",		required_argument,		NULL,		'm' },
			{"max-version",		required_argument,		NULL,		'M' },
			{"version",			required_argument,		NULL,		'v' },
			{"auth-optional",	no_argument,			NULL,		'n' },
			{"identity",		required_argument,		NULL,		'i' },
			{"debug",			required_argument,		NULL,		'd' },
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
			case 'f':
				if (!load_certificate(optarg))
				{
					return 1;
				}
				client = identification_create_from_encoding(ID_ANY, chunk_empty);
				continue;
			case 'i':
				identity = identification_create_from_string(optarg);
				if (!identity)
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
			case '4':
				family = AF_INET;
				continue;
			case '6':
				family = AF_INET6;
				continue;
			case 'm':
				if (!enum_from_name(tls_numeric_version_names, optarg,
									&min_version))
				{
					fprintf(stderr, "unknown minimum TLS version: %s\n", optarg);
					return 1;
				}
				continue;
			case 'M':
				if (!enum_from_name(tls_numeric_version_names, optarg,
									&max_version))
				{
					fprintf(stderr, "unknown maximum TLS version: %s\n", optarg);
					return 1;
				}
				continue;
			case 'v':
				if (!enum_from_name(tls_numeric_version_names, optarg,
									&min_version))
				{
					fprintf(stderr, "unknown TLS version: %s\n", optarg);
					return 1;
				}
				max_version = min_version;
				continue;
			case 'n':
				flags |= TLS_FLAG_CLIENT_AUTH_OPTIONAL;
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
	host = host_create_from_dns(address, family, port);
	if (!host)
	{
		DBG1(DBG_TLS, "resolving hostname %s failed", address);
		return 1;
	}
	server = identification_create_from_string(address);
	cache = tls_cache_create(100, 30);
	if (listen)
	{
		res = serve(host, server, identity ?: client, times, cache, min_version,
					max_version, flags);
	}
	else
	{
		DESTROY_IF(client);
		client = find_client_id();
		res = run_client(host, identity ?: server, client, times, cache, min_version,
						 max_version, flags);
		DESTROY_IF(client);
	}
	cache->destroy(cache);
	host->destroy(host);
	server->destroy(server);
	DESTROY_IF(identity);
	return res;
}
