/*
 * Copyright (C) 2011-2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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

#define _GNU_SOURCE
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#ifdef HAVE_SYSLOG
# include <syslog.h>
#endif

#include <library.h>
#include <utils/debug.h>

#include <imcv.h>
#include <pts/pts_meas_algo.h>

#include "attest_db.h"
#include "attest_usage.h"

/**
 * global debug output variables
 */
static int debug_level = 1;
static bool stderr_quiet = TRUE;

/**
 * attest dbg function
 */
static void attest_dbg(debug_t group, level_t level, char *fmt, ...)
{
	va_list args;

	if (level <= debug_level)
	{
		if (!stderr_quiet)
		{
			va_start(args, fmt);
			vfprintf(stderr, fmt, args);
			fprintf(stderr, "\n");
			va_end(args);
		}

#ifdef HAVE_SYSLOG
		{
			int priority = LOG_INFO;
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
				syslog(priority, "%s\n", current);
				current = next;
			}
		}
#endif /* HAVE_SYSLOG */
	}
}

/**
 * global attestation database object
 */
attest_db_t *attest;


/**
 * atexit handler to close db on shutdown
 */
static void cleanup(void)
{
	attest->destroy(attest);
	libimcv_deinit();
#ifdef HAVE_SYSLOG
	closelog();
#endif
}

static void do_args(int argc, char *argv[])
{
	enum {
		OP_UNDEF,
		OP_USAGE,
		OP_KEYS,
		OP_COMPONENTS,
		OP_DEVICES,
		OP_DIRECTORIES,
		OP_FILES,
		OP_HASHES,
		OP_MEASUREMENTS,
		OP_PACKAGES,
		OP_PRODUCTS,
		OP_SESSIONS,
		OP_ADD,
		OP_DEL,
	} op = OP_UNDEF;

	/* reinit getopt state */
	optind = 0;

	while (TRUE)
	{
		int c;

		struct option long_opts[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "components", no_argument, NULL, 'c' },
			{ "devices", no_argument, NULL, 'e' },
			{ "directories", no_argument, NULL, 'd' },
			{ "dirs", no_argument, NULL, 'd' },
			{ "files", no_argument, NULL, 'f' },
			{ "keys", no_argument, NULL, 'k' },
			{ "packages", no_argument, NULL, 'g' },
			{ "products", no_argument, NULL, 'p' },
			{ "hashes", no_argument, NULL, 'H' },
			{ "measurements", no_argument, NULL, 'm' },
			{ "sessions", no_argument, NULL, 's' },
			{ "add", no_argument, NULL, 'a' },
			{ "delete", no_argument, NULL, 'r' },
			{ "del", no_argument, NULL, 'r' },
			{ "remove", no_argument, NULL, 'r' },
			{ "aik", required_argument, NULL, 'A' },
			{ "blacklist", no_argument, NULL, 'B' },
			{ "component", required_argument, NULL, 'C' },
			{ "comp", required_argument, NULL, 'C' },
			{ "directory", required_argument, NULL, 'D' },
			{ "dir", required_argument, NULL, 'D' },
			{ "file", required_argument, NULL, 'F' },
			{ "package", required_argument, NULL, 'G' },
			{ "key", required_argument, NULL, 'K' },
			{ "measdir", required_argument, NULL, 'M' },
			{ "owner", required_argument, NULL, 'O' },
			{ "product", required_argument, NULL, 'P' },
			{ "relative", no_argument, NULL, 'R' },
			{ "rel", no_argument, NULL, 'R' },
			{ "sequence", required_argument, NULL, 'S' },
			{ "seq", required_argument, NULL, 'S' },
			{ "utc", no_argument, NULL, 'U' },
			{ "version", required_argument, NULL, 'V' },
			{ "security", no_argument, NULL, 'Y' },
			{ "sha1", no_argument, NULL, '1' },
			{ "sha256", no_argument, NULL, '2' },
			{ "sha384", no_argument, NULL, '3' },
			{ "did", required_argument, NULL, '4' },
			{ "fid", required_argument, NULL, '5' },
			{ "pid", required_argument, NULL, '6' },
			{ "cid", required_argument, NULL, '7' },
			{ "kid", required_argument, NULL, '8' },
			{ "gid", required_argument, NULL, '9' },
			{ 0,0,0,0 }
		};

		c = getopt_long(argc, argv, "", long_opts, NULL);
		switch (c)
		{
			case EOF:
				break;
			case 'h':
				op = OP_USAGE;
				break;
			case 'c':
				op = OP_COMPONENTS;
				continue;
			case 'd':
				op = OP_DIRECTORIES;
				continue;
			case 'e':
				op = OP_DEVICES;
				continue;
			case 'f':
				op = OP_FILES;
				continue;
			case 'g':
				op = OP_PACKAGES;
				continue;
			case 'k':
				op = OP_KEYS;
				continue;
			case 'p':
				op = OP_PRODUCTS;
				continue;
			case 'H':
				op = OP_HASHES;
				continue;
			case 'm':
				op = OP_MEASUREMENTS;
				continue;
			case 's':
				op = OP_SESSIONS;
				continue;
			case 'a':
				op = OP_ADD;
				continue;
			case 'r':
				op = OP_DEL;
				continue;
			case 'A':
			{
				certificate_t *aik_cert;
				public_key_t *aik_key;
				chunk_t aik;

				aik_cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
								CERT_X509, BUILD_FROM_FILE, optarg, BUILD_END);
				if (!aik_cert)
				{
					printf("AIK certificate '%s' could not be loaded\n", optarg);
					exit(EXIT_FAILURE);
				}
				aik_key = aik_cert->get_public_key(aik_cert);
				aik_cert->destroy(aik_cert);

				if (!aik_key)
				{
					printf("AIK public key could not be retrieved\n");
					exit(EXIT_FAILURE);
				}
				if (!aik_key->get_fingerprint(aik_key, KEYID_PUBKEY_INFO_SHA1,
											  &aik))
				{
					printf("AIK fingerprint could not be computed\n");
					aik_key->destroy(aik_key);
					exit(EXIT_FAILURE);
				}
				aik = chunk_clone(aik);
				aik_key->destroy(aik_key);

				if (!attest->set_key(attest, aik, op == OP_ADD))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			}
			case 'B':
				attest->set_package_state(attest, OS_PACKAGE_STATE_BLACKLIST);
				continue;
			case 'C':
				if (!attest->set_component(attest, optarg, op == OP_ADD))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case 'D':
				if (!attest->set_directory(attest, optarg, op == OP_ADD))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case 'F':
			{
				char *dir = path_dirname(optarg);
				char *file = path_basename(optarg);

				if (*dir != '.')
				{
					if (!attest->set_directory(attest, dir, op == OP_ADD))
					{
						free(file);
						free(dir);
						exit(EXIT_FAILURE);
					}
				}
				free(dir);

				if (!attest->set_file(attest, file, op == OP_ADD))
				{
					free(file);
					exit(EXIT_FAILURE);
				}
				free(file);
				continue;
			}
			case 'G':
				if (!attest->set_package(attest, optarg, op == OP_ADD))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case 'K':
			{
				chunk_t aik;

				aik = chunk_from_hex(chunk_create(optarg, strlen(optarg)), NULL);
				if (!attest->set_key(attest, aik, op == OP_ADD))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			}
			case 'M':
				if (!attest->set_meas_directory(attest, optarg))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case 'O':
				attest->set_owner(attest, optarg);
				continue;
			case 'P':
				if (!attest->set_product(attest, optarg, op == OP_ADD))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case 'R':
				attest->set_relative(attest);
				continue;
			case 'S':
				attest->set_sequence(attest, atoi(optarg));
				continue;
			case 'U':
				attest->set_utc(attest);
				continue;
			case 'V':
				if (!attest->set_version(attest, optarg))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case 'Y':
				attest->set_package_state(attest, OS_PACKAGE_STATE_SECURITY);
				continue;
			case '1':
				attest->set_algo(attest, PTS_MEAS_ALGO_SHA1);
				continue;
			case '2':
				attest->set_algo(attest, PTS_MEAS_ALGO_SHA256);
				continue;
			case '3':
				attest->set_algo(attest, PTS_MEAS_ALGO_SHA384);
				continue;
			case '4':
				if (!attest->set_did(attest, atoi(optarg)))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case '5':
				if (!attest->set_fid(attest, atoi(optarg)))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case '6':
				if (!attest->set_pid(attest, atoi(optarg)))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case '7':
				if (!attest->set_cid(attest, atoi(optarg)))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case '8':
				if (!attest->set_kid(attest, atoi(optarg)))
				{
					exit(EXIT_FAILURE);
				}
				continue;
			case '9':
				if (!attest->set_gid(attest, atoi(optarg)))
				{
					exit(EXIT_FAILURE);
				}
				continue;
		}
		break;
	}

	switch (op)
	{
		case OP_USAGE:
			usage();
			break;
		case OP_PACKAGES:
			attest->list_packages(attest);
			break;
		case OP_PRODUCTS:
			attest->list_products(attest);
			break;
		case OP_KEYS:
			attest->list_keys(attest);
			break;
		case OP_COMPONENTS:
			attest->list_components(attest);
			break;
		case OP_DEVICES:
			attest->list_devices(attest);
			break;
		case OP_DIRECTORIES:
			attest->list_directories(attest);
			break;
		case OP_FILES:
			attest->list_files(attest);
			break;
		case OP_HASHES:
			attest->list_hashes(attest);
			break;
		case OP_MEASUREMENTS:
			attest->list_measurements(attest);
			break;
		case OP_SESSIONS:
			attest->list_sessions(attest);
			break;
		case OP_ADD:
			attest->add(attest);
			break;
		case OP_DEL:
			attest->delete(attest);
			break;
		default:
			usage();
			exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	char *uri;

	/* enable attest debugging hook */
	dbg = attest_dbg;
#ifdef HAVE_SYSLOG
	openlog("attest", 0, LOG_DEBUG);
#endif

	atexit(library_deinit);

	/* initialize library */
	if (!library_init(NULL, "attest"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "attest.load", PLUGINS)))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	uri = lib->settings->get_str(lib->settings, "attest.database", NULL);
	if (!uri)
	{
		fprintf(stderr, "database URI attest.database not set.\n");
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	attest = attest_db_create(uri);
	if (!attest)
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	atexit(cleanup);
	libimcv_init(FALSE);

	do_args(argc, argv);

	exit(EXIT_SUCCESS);
}
