/*
 * Copyright (C) 2014-2016 Andreas Steffen
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

#include "tpm_tss.h"

#include <library.h>
#include <utils/debug.h>
#include <utils/optionsfrom.h>
#include <credentials/certificates/x509.h>
#include <credentials/keys/public_key.h>

#include <syslog.h>
#include <getopt.h>
#include <errno.h>

/* default directory where AIK keys are stored */
#define AIK_DIR							IPSEC_CONFDIR "/pts/"

/* default name of AIK private key blob */
#define DEFAULT_FILENAME_AIKBLOB		AIK_DIR "aikBlob.bin"

/* default name of AIK public key */
#define DEFAULT_FILENAME_AIKPUBKEY		AIK_DIR "aikPub.der"

/* logging */
static bool log_to_stderr = TRUE;
static bool log_to_syslog = TRUE;
static level_t default_loglevel = 1;

/* options read by optionsfrom */
options_t *options;

/* global variables */
certificate_t *cacert;
public_key_t *ca_pubkey;
chunk_t ca_modulus;
chunk_t aik_pubkey;
chunk_t aik_keyid;
tpm_tss_t *tpm;

/**
 * logging function for aikgen
 */
static void aikgen_dbg(debug_t group, level_t level, char *fmt, ...)
{
	char buffer[8192];
	char *current = buffer, *next;
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
		if (log_to_syslog)
		{
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
	}
}

/**
 * Initialize logging to stderr/syslog
 */
static void init_log(const char *program)
{
	dbg = aikgen_dbg;

	if (log_to_stderr)
	{
		setbuf(stderr, NULL);
	}
	if (log_to_syslog)
	{
		openlog(program, LOG_CONS | LOG_NDELAY | LOG_PID, LOG_AUTHPRIV);
	}
}

/**
 * @brief exit aikgen
 *
 * @param status 0 = OK, -1 = general discomfort
 */
static void exit_aikgen(err_t message, ...)
{
	int status = 0;

	DESTROY_IF(tpm);
	DESTROY_IF(cacert);
	DESTROY_IF(ca_pubkey);
	free(ca_modulus.ptr);
	free(aik_pubkey.ptr);
	free(aik_keyid.ptr);
	options->destroy(options);

	/* print any error message to stderr */
	if (message != NULL && *message != '\0')
	{
		va_list args;
		char m[8192];

		va_start(args, message);
		vsnprintf(m, sizeof(m), message, args);
		va_end(args);

		fprintf(stderr, "aikgen error: %s\n", m);
		status = -1;
	}
	library_deinit();
	exit(status);
}

/**
 * @brief prints the usage of the program to the stderr output
 *
 * If message is set, program is exited with 1 (error)
 * @param message message in case of an error
 */
static void usage(const char *message)
{
	fprintf(stderr,
		"Usage: aikgen  --cacert|capubkey <filename>"
		" [--aikblob <filename>] [--aikpubkey <filename>] \n"
		"              [--idreq <filename>] [--force]"
		" [--quiet] [--debug <level>]\n"
		"       aikgen  --help\n"
		"\n"
		"Options:\n"
		" --cacert (-c)     certificate of [privacy] CA\n"
		" --capubkey (-k)   public key of [privacy] CA\n"
		" --aikblob (-b)    encrypted blob with AIK private key\n"
		" --aikpubkey (-p)  AIK public key\n"
		" --idreq (-i)      encrypted identity request\n"
		" --force (-f)      force to overwrite existing files\n"
		" --help (-h)       show usage and exit\n"
		"\n"
		"Debugging output:\n"
		" --debug (-l)      changes the log level (-1..4, default: 1)\n"
		" --quiet (-q)      do not write log output to stderr\n"
		);
	exit_aikgen(message);
}

/**
 * @brief main of aikgen which generates an Attestation Identity Key (AIK)
 *
 * @param argc number of arguments
 * @param argv pointer to the argument values
 */
int main(int argc, char *argv[])
{
	/* external values */
	extern char * optarg;
	extern int optind;

	char *cacert_filename    = NULL;
	char *capubkey_filename  = NULL;
	char *aikblob_filename   = DEFAULT_FILENAME_AIKBLOB;
	char *aikpubkey_filename = DEFAULT_FILENAME_AIKPUBKEY;
	char *idreq_filename     = NULL;
	bool force = FALSE;
	chunk_t identity_req;
	chunk_t aik_blob;
	hasher_t *hasher;

	atexit(library_deinit);
	if (!library_init(NULL, "aikgen"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "aikgen", argv[0]))
	{
		fprintf(stderr, "integrity check of aikgen failed\n");
		exit(SS_RC_DAEMON_INTEGRITY);
	}

	/* initialize global variables */
	options = options_create();

	for (;;)
	{
		static const struct option long_opts[] = {
			/* name, has_arg, flag, val */
			{ "help", no_argument, NULL, 'h' },
			{ "optionsfrom", required_argument, NULL, '+' },
			{ "cacert", required_argument, NULL, 'c' },
			{ "capubkey", required_argument, NULL, 'k' },
			{ "aikblob", required_argument, NULL, 'b' },
			{ "aikpubkey", required_argument, NULL, 'p' },
			{ "idreq", required_argument, NULL, 'i' },
			{ "force", no_argument, NULL, 'f' },
			{ "quiet", no_argument, NULL, 'q' },
			{ "debug", required_argument, NULL, 'l' },
			{ 0,0,0,0 }
		};

		/* parse next option */
		int c = getopt_long(argc, argv, "ho:c:b:p:fqd:", long_opts, NULL);

		switch (c)
		{
			case EOF:       /* end of flags */
				break;

			case 'h':       /* --help */
				usage(NULL);

			case '+':       /* --optionsfrom <filename> */
				if (!options->from(options, optarg, &argc, &argv, optind))
				{
					exit_aikgen("optionsfrom failed");
				}
				continue;

			case 'c':       /* --cacert <filename> */
				cacert_filename = optarg;
				continue;

			case 'k':       /* --capubkey <filename> */
				capubkey_filename = optarg;
				continue;

			case 'b':       /* --aikblob <filename> */
				aikblob_filename = optarg;
				continue;

			case 'p':       /* --aikpubkey <filename> */
				aikpubkey_filename = optarg;
				continue;

			case 'i':       /* --idreq <filename> */
				idreq_filename = optarg;
				continue;

			case 'f':       /* --force */
				force = TRUE;
				continue;

			case 'q':       /* --quiet */
				log_to_stderr = FALSE;
				continue;

			case 'l':		/* --debug <level> */
				default_loglevel = atoi(optarg);
				continue;

			default:
				usage("unknown option");
		}
		/* break from loop */
		break;
	}

	init_log("aikgen");

	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "aikgen.load", PLUGINS)))
	{
		exit_aikgen("plugin loading failed");
	}

	/* read certificate of [privacy] CA if it exists */
	if (cacert_filename)
	{
		cacert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								BUILD_FROM_FILE, cacert_filename, BUILD_END);
		if (!cacert)
		{
			exit_aikgen("could not read ca certificate file '%s'",
						 cacert_filename);
		}
	}

	/* optionally read public key of [privacy CA] if it exists */
	if (!cacert)
	{
		if (!capubkey_filename)
		{
			usage("either --cacert or --capubkey option is required");
		}
		cacert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
								CERT_TRUSTED_PUBKEY, BUILD_FROM_FILE,
								capubkey_filename, BUILD_END);
		if (!cacert)
		{
			exit_aikgen("could not read ca public key file '%s'",
						 capubkey_filename);
		}
	}

	/* extract public key from CA certificate or trusted CA public key */
	ca_pubkey = cacert->get_public_key(cacert);
	if (!ca_pubkey)
	{
		exit_aikgen("could not extract ca public key");
	}
	if (ca_pubkey->get_type(ca_pubkey) != KEY_RSA ||
		ca_pubkey->get_keysize(ca_pubkey) != 2048)
	{
		exit_aikgen("CA public key must be RSA 2048 but is %N %d",
					 key_type_names, ca_pubkey->get_type(ca_pubkey),
					 ca_pubkey->get_keysize(ca_pubkey));
	}
	if (!ca_pubkey->get_encoding(ca_pubkey, PUBKEY_RSA_MODULUS, &ca_modulus))
	{
		exit_aikgen("could not extract RSA modulus from CA public key");
	}

	/* try to find a TPM 1.2 */
	tpm = tpm_tss_probe(TPM_VERSION_1_2);
	if (!tpm)
	{
		exit_aikgen("no TPM 1.2 found");
	}

	if (!tpm->generate_aik(tpm, ca_modulus, &aik_blob, &aik_pubkey,
						   &identity_req))
	{
		exit_aikgen("could not generate AIK");
	}

	/* optionally output identity request encrypted with CA public key */
	if (idreq_filename)
	{
		if (!chunk_write(identity_req, idreq_filename, 0022, force))
		{
			exit_aikgen("could not write AIK identity request file '%s': %s",
						 idreq_filename, strerror(errno));
		}
		DBG1(DBG_LIB, "AIK identity request written to '%s' (%u bytes)",
					   idreq_filename, identity_req.len);
	}

	/* output AIK private key blob */
	if (!chunk_write(aik_blob, aikblob_filename, 0022, force))
	{
		exit_aikgen("could not write AIK blob file '%s': %s",
					 aikblob_filename, strerror(errno));
	}
	DBG1(DBG_LIB, "AIK private key blob written to '%s' (%u bytes)",
				   aikblob_filename, aik_blob.len);

	/* output AIK public key */
	if (!chunk_write(aik_pubkey, aikpubkey_filename, 0022, force))
	{
		exit_aikgen("could not write AIK public key file '%s': %s",
					 aikpubkey_filename, strerror(errno));
	}
	DBG1(DBG_LIB, "AIK public key written to '%s' (%u bytes)",
				   aikpubkey_filename, aik_pubkey.len);

	/* display AIK keyid derived from subjectPublicKeyInfo encoding */
	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher || !hasher->allocate_hash(hasher, aik_pubkey, &aik_keyid))
	{
		DESTROY_IF(hasher);
		exit_aikgen("SHA1 hash algorithm not supported, computation of AIK "
					"keyid failed");
	}
	hasher->destroy(hasher);
	DBG1(DBG_LIB, "AIK keyid: %#B", &aik_keyid);

	exit_aikgen(NULL);
	return -1; /* should never be reached */
}
