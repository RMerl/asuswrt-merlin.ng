/*
 * Copyright (C) 2017 Andreas Steffen
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

#include <tpm_tss.h>

#include <library.h>
#include <crypto/hashers/hasher.h>
#include <utils/debug.h>

#include <syslog.h>
#include <getopt.h>
#include <errno.h>


/* logging */
static bool log_to_stderr = TRUE;
static bool log_to_syslog = TRUE;
static level_t default_loglevel = 1;

/* global variables */
tpm_tss_t *tpm;
chunk_t digest;
chunk_t pcr_value;

/**
 * logging function for tpm_extendpcr
 */
static void tpm_extendpcr_dbg(debug_t group, level_t level, char *fmt, ...)
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
	dbg = tpm_extendpcr_dbg;

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
 * @brief exit tpm_extendpcr
 *
 * @param status 0 = OK, -1 = general discomfort
 */
static void exit_tpm_extendpcr(err_t message, ...)
{
	int status = 0;

	DESTROY_IF(tpm);
	chunk_free(&digest);
	chunk_free(&pcr_value);

	/* print any error message to stderr */
	if (message != NULL && *message != '\0')
	{
		va_list args;
		char m[8192];

		va_start(args, message);
		vsnprintf(m, sizeof(m), message, args);
		va_end(args);

		fprintf(stderr, "tpm_extendpcr error: %s\n", m);
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
		"Usage: tpm_extendpcr [--alg <name>] --pcr <nr> --digest <hex>|--in"
		" <file>\n"
		"                     [--hash] [--out <file>] [--quiet]"
		" [--debug <level>]\n"
		"       tpm_extendpcr  --help\n"
		"\n"
		"Options:\n"
		" --alg (-a)     hash algorithm (sha1|sha256)\n"
		" --pcr (-p)     platform configuration register (0..23)\n"
		" --digest (-d)  digest in hex format to be extended\n"
		" --in (-i)      binary input file with digest to be extended\n"
		" --hash (-x)    prehash the input file to create digest\n"
		" --out (-o)     binary output file with updated PCR value\n"
		" --help (-h)    show usage and exit\n"
		"\n"
		"Debugging output:\n"
		" --debug (-l)   changes the log level (-1..4, default: 1)\n"
		" --quiet (-q)   do not write log output to stderr\n"
		);
	exit_tpm_extendpcr(message);
}

/**
 * @brief main of tpm_extendpcr which extends digest into a PCR
 *
 * @param argc number of arguments
 * @param argv pointer to the argument values
 */
int main(int argc, char *argv[])
{
	hash_algorithm_t alg = HASH_SHA1;
	hasher_t *hasher = NULL;
	char *infile = NULL, *outfile = NULL;
	uint32_t pcr = 16;
	bool hash = FALSE;

	atexit(library_deinit);
	if (!library_init(NULL, "tpm_extendpcr"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "tpm_extendpcr", argv[0]))
	{
		fprintf(stderr, "integrity check of tpm_extendpcr failed\n");
		exit(SS_RC_DAEMON_INTEGRITY);
	}

	for (;;)
	{
		static const struct option long_opts[] = {
			/* name, has_arg, flag, val */
			{ "help", no_argument, NULL, 'h' },
			{ "alg", required_argument, NULL, 'a' },
			{ "pcr", required_argument, NULL, 'p' },
			{ "digest", required_argument, NULL, 'd' },
			{ "in", required_argument, NULL, 'i' },
			{ "hash", no_argument, NULL, 'x' },
			{ "out", required_argument, NULL, 'o' },
			{ "quiet", no_argument, NULL, 'q' },
			{ "debug", required_argument, NULL, 'l' },
			{ 0,0,0,0 }
		};

		/* parse next option */
		int c = getopt_long(argc, argv, "ha:p:d:i:xo:ql:", long_opts, NULL);

		switch (c)
		{
			case EOF:       /* end of flags */
				break;

			case 'h':       /* --help */
				usage(NULL);

			case 'a':       /* --alg <name> */
				if (!enum_from_name(hash_algorithm_short_names, optarg, &alg))
				{
					usage("unsupported hash algorithm");
				}
				continue;
			case 'p':       /* --pcr <nr> */
				pcr = atoi(optarg);
				continue;

			case 'd':       /* --digest <hex> */
				digest = chunk_from_hex(chunk_from_str(optarg), NULL);
				continue;

			case 'i':       /* --in <file> */
				infile = optarg;
				continue;

			case 'x':       /* --hash */
				hash = TRUE;
				continue;

			case 'o':       /* --out <file> */
				outfile = optarg;
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

	init_log("tpm_extendpcr");

	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "tpm_extendpcr.load",
												  "tpm sha1 sha2")))
	{
		exit_tpm_extendpcr("plugin loading failed");
	}

	/* try to find a TPM */
	tpm = tpm_tss_probe(TPM_VERSION_ANY);
	if (!tpm)
	{
		exit_tpm_extendpcr("no TPM found");
	}

	/* read digest from file */
	if (digest.len == 0)
	{
		chunk_t *chunk;

		if (!infile)
		{
			exit_tpm_extendpcr("--digest or --in option required");
		}
		chunk = chunk_map(infile, FALSE);
		if (!chunk)
		{
			exit_tpm_extendpcr("reading input file failed");
		}
		if (hash)
		{
			hasher = lib->crypto->create_hasher(lib->crypto, alg);
			if (!hasher || !hasher->allocate_hash(hasher, *chunk, &digest))
			{
				DESTROY_IF(hasher);
				chunk_unmap(chunk);
				exit_tpm_extendpcr("prehashing infile failed");
			}
			hasher->destroy(hasher);
		}
		else
		{
			digest = chunk_clone(*chunk);
		}
		chunk_unmap(chunk);
	}
	DBG1(DBG_PTS, "Digest: %#B", &digest);

	/* extend digest into PCR */
	if (!tpm->extend_pcr(tpm, pcr, &pcr_value, digest, alg))
	{
		exit_tpm_extendpcr("extending PCR failed");
	}
	DBG1(DBG_PTS, "PCR %02u: %#B", pcr, &pcr_value);

	/* write PCR value to file */
	if (outfile)
	{
		if (!chunk_write(pcr_value, outfile, 022, TRUE))
		{
			DBG1(DBG_PTS, "writing '%s' failed", outfile);
		}
	}
	chunk_free(&pcr_value);

	exit_tpm_extendpcr(NULL);
	return -1; /* should never be reached */
}
