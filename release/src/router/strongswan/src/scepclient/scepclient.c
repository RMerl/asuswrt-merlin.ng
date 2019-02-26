/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005 Jan Hutter, Martin Willi
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <syslog.h>
#include <errno.h>

#include <library.h>
#include <utils/debug.h>
#include <asn1/asn1.h>
#include <asn1/oid.h>
#include <utils/optionsfrom.h>
#include <collections/enumerator.h>
#include <collections/linked_list.h>
#include <crypto/hashers/hasher.h>
#include <crypto/crypters/crypter.h>
#include <crypto/proposal/proposal_keywords.h>
#include <credentials/keys/private_key.h>
#include <credentials/keys/public_key.h>
#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>
#include <credentials/certificates/pkcs10.h>
#include <credentials/sets/mem_cred.h>
#include <plugins/plugin.h>

#include "scep.h"

/*
 * definition of some defaults
 */

/* some paths */
#define REQ_PATH                        IPSEC_CONFDIR "/ipsec.d/reqs"
#define HOST_CERT_PATH                  IPSEC_CONFDIR "/ipsec.d/certs"
#define CA_CERT_PATH                    IPSEC_CONFDIR "/ipsec.d/cacerts"
#define PRIVATE_KEY_PATH                IPSEC_CONFDIR "/ipsec.d/private"

/* default name of DER-encoded PKCS#1 private key file */
#define DEFAULT_FILENAME_PKCS1          "myKey.der"

/* default name of DER-encoded PKCS#10 certificate request file */
#define DEFAULT_FILENAME_PKCS10         "myReq.der"

/* default name of DER-encoded PKCS#7 file */
#define DEFAULT_FILENAME_PKCS7          "pkcs7.der"

/* default name of DER-encoded self-signed X.509 certificate file */
#define DEFAULT_FILENAME_CERT_SELF      "selfCert.der"

/* default name of DER-encoded X.509 certificate file */
#define DEFAULT_FILENAME_CERT           "myCert.der"

/* default name of DER-encoded CA cert file used for key encipherment */
#define DEFAULT_FILENAME_CACERT_ENC     "caCert.der"

/* default name of the der encoded CA cert file used for signature verification */
#define DEFAULT_FILENAME_CACERT_SIG     "caCert.der"

/* default prefix of the der encoded CA certificates received from the SCEP server */
#define DEFAULT_FILENAME_PREFIX_CACERT  "caCert.der"

/* default certificate validity */
#define DEFAULT_CERT_VALIDITY    5 * 3600 * 24 * 365  /* seconds */

/* default polling time interval in SCEP manual mode */
#define DEFAULT_POLL_INTERVAL    20       /* seconds */

/* default key length for self-generated RSA keys */
#define DEFAULT_RSA_KEY_LENGTH 2048       /* bits */

/* default distinguished name */
#define DEFAULT_DN "C=CH, O=Linux strongSwan, CN="

/* minimum RSA key size */
#define RSA_MIN_OCTETS (512 / BITS_PER_BYTE)

/* challenge password buffer size */
#define MAX_PASSWORD_LENGTH 256

/* Max length of filename for tempfile */
#define MAX_TEMP_FILENAME_LENGTH 256


/* current scepclient version */
static const char *scepclient_version = "1.0";

/* by default the CRL policy is lenient */
bool strict_crl_policy = FALSE;

/* by default pluto does not check crls dynamically */
long crl_check_interval = 0;

/* by default pluto logs out after every smartcard use */
bool pkcs11_keep_state = FALSE;

/* by default HTTP fetch timeout is 30s */
static u_int http_timeout = 30;

/* address to bind for HTTP fetches */
static char* http_bind = NULL;

/* options read by optionsfrom */
options_t *options;

/*
 * Global variables
 */
chunk_t pkcs1;
chunk_t pkcs7;
chunk_t challengePassword;
chunk_t serialNumber;
chunk_t transID;
chunk_t fingerprint;
chunk_t encoding;
chunk_t pkcs10_encoding;
chunk_t issuerAndSubject;
chunk_t getCertInitial;
chunk_t scep_response;

linked_list_t *subjectAltNames;

identification_t *subject      = NULL;
private_key_t *private_key     = NULL;
public_key_t *public_key       = NULL;
certificate_t *x509_signer     = NULL;
certificate_t *x509_ca_enc     = NULL;
certificate_t *x509_ca_sig     = NULL;
certificate_t *pkcs10_req      = NULL;

mem_cred_t *creds              = NULL;

/* logging */
static bool log_to_stderr = TRUE;
static bool log_to_syslog = TRUE;
static level_t default_loglevel = 1;

/**
 * logging function for scepclient
 */
static void scepclient_dbg(debug_t group, level_t level, char *fmt, ...)
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
	dbg = scepclient_dbg;

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
 * join two paths if filename is not absolute
 */
static void join_paths(char *target, size_t target_size, char *parent,
					   char *filename)
{
	if (*filename == '/' || *filename == '.')
	{
		snprintf(target, target_size, "%s", filename);
	}
	else
	{
		snprintf(target, target_size, "%s/%s", parent, filename);
	}
}

/**
 * add a suffix to a given filename, properly handling extensions like '.der'
 */
static void add_path_suffix(char *target, size_t target_size, char *filename,
							char *suffix_fmt, ...)
{
	char suffix[PATH_MAX], *start, *dot;
	va_list args;

	va_start(args, suffix_fmt);
	vsnprintf(suffix, sizeof(suffix), suffix_fmt, args);
	va_end(args);

	start = strrchr(filename, '/');
	start = start ?: filename;
	dot = strrchr(start, '.');

	if (!dot || dot == start || dot[1] == '\0')
	{	/* no extension add suffix at the end */
		snprintf(target, target_size, "%s%s", filename, suffix);
	}
	else
	{	/* add the suffix between the filename and the extension */
		snprintf(target, target_size, "%.*s%s%s", (int)(dot - filename),
				 filename, suffix, dot);
	}
}

/**
 * @brief exit scepclient
 *
 * @param status 0 = OK, 1 = general discomfort
 */
static void exit_scepclient(err_t message, ...)
{
	int status = 0;

	if (creds)
	{
		lib->credmgr->remove_set(lib->credmgr, &creds->set);
		creds->destroy(creds);
	}

	DESTROY_IF(subject);
	DESTROY_IF(private_key);
	DESTROY_IF(public_key);
	DESTROY_IF(x509_signer);
	DESTROY_IF(x509_ca_enc);
	DESTROY_IF(x509_ca_sig);
	DESTROY_IF(pkcs10_req);
	subjectAltNames->destroy_offset(subjectAltNames,
								   offsetof(identification_t, destroy));
	free(pkcs1.ptr);
	free(pkcs7.ptr);
	free(serialNumber.ptr);
	free(transID.ptr);
	free(fingerprint.ptr);
	free(encoding.ptr);
	free(pkcs10_encoding.ptr);
	free(issuerAndSubject.ptr);
	free(getCertInitial.ptr);
	free(scep_response.ptr);
	options->destroy(options);

	/* print any error message to stderr */
	if (message != NULL && *message != '\0')
	{
		va_list args;
		char m[8192];

		va_start(args, message);
		vsnprintf(m, sizeof(m), message, args);
		va_end(args);

		fprintf(stderr, "error: %s\n", m);
		status = -1;
	}
	library_deinit();
	exit(status);
}

/**
 * @brief prints the program version and exits
 *
 */
static void version(void)
{
	printf("scepclient %s\n", scepclient_version);
	exit_scepclient(NULL);
}

/**
 * @brief prints the usage of the program to the stderr output
 *
 * If message is set, program is exitet with 1 (error)
 * @param message message in case of an error
 */
static void usage(const char *message)
{
	fprintf(stderr,
		"Usage: scepclient\n"
		" --help (-h)                       show usage and exit\n"
		" --version (-v)                    show version and exit\n"
		" --quiet (-q)                      do not write log output to stderr\n"
		" --in (-i) <type>[=<filename>]     use <filename> of <type> for input\n"
		"                                   <type> = pkcs1 | pkcs10 | cert-self\n"
		"                                            cacert-enc | cacert-sig\n"
		"                                   - if no pkcs1 input is defined, an RSA\n"
		"                                     key will be generated\n"
		"                                   - if no pkcs10 input is defined, a\n"
		"                                     PKCS#10 request will be generated\n"
		"                                   - if no cert-self input is defined, a\n"
		"                                     self-signed certificate will be generated\n"
		"                                   - if no filename is given, default is used\n"
		" --out (-o) <type>[=<filename>]    write output of <type> to <filename>\n"
		"                                   multiple outputs are allowed\n"
		"                                   <type> = pkcs1 | pkcs10 | pkcs7 | cert-self |\n"
		"                                            cert | cacert\n"
		"                                   - type cacert defines filename prefix of\n"
		"                                     received CA certificate(s)\n"
		"                                   - if no filename is given, default is used\n"
		" --optionsfrom (-+) <filename>     reads additional options from given file\n"
		" --force (-f)                      force existing file(s)\n"
		" --httptimeout (-T)                timeout for HTTP operations (default: 30s)\n"
		" --bind (-b)                       source address to bind for HTTP operations\n"
		"\n"
		"Options for key generation (pkcs1):\n"
		" --keylength (-k) <bits>           key length for RSA key generation\n"
		"                                   (default: 2048 bits)\n"
		"\n"
		"Options for validity:\n"
		" --days (-D) <days>                validity in days\n"
		" --startdate (-S) <YYMMDDHHMMSS>Z  not valid before date\n"
		" --enddate   (-E) <YYMMDDHHMMSS>Z  not valid after date\n"
		"\n"
		"Options for request generation (pkcs10):\n"
		" --dn (-d) <dn>                    comma separated list of distinguished names\n"
		" --subjectAltName (-s) <t>=<v>     include subjectAltName in certificate request\n"
		"                                   <t> =  email | dns | ip \n"
		" --password (-p) <pw>              challenge password\n"
		"                                   - use '%%prompt' as pw for a password prompt\n"
		" --algorithm (-a) [<type>=]<algo>  algorithm to be used for PKCS#7 encryption,\n"
		"                                   PKCS#7 digest or PKCS#10 signature\n"
		"                                   <type> = enc | dgst | sig\n"
		"                                   - if no type is given enc is assumed\n"
		"                                   <algo> = des (default) | 3des | aes128 |\n"
		"                                            aes192 | aes256 | camellia128 |\n"
		"                                            camellia192 | camellia256\n"
		"                                   <algo> = md5 (default) | sha1 | sha256 |\n"
		"                                            sha384 | sha512\n"
		"\n"
		"Options for CA certificate acquisition:\n"
		" --caname (-c) <name>              name of CA to fetch CA certificate(s)\n"
		"                                   (default: CAIdentifier)\n"
		"Options for enrollment (cert):\n"
		" --url (-u) <url>                  url of the SCEP server\n"
		" --method (-m) post | get          http request type\n"
		" --interval (-t) <seconds>         poll interval in seconds (default 20s)\n"
		" --maxpolltime (-x) <seconds>      max poll time in seconds when in manual mode\n"
		"                                   (default: unlimited)\n"
		"\n"
		"Debugging output:\n"
		" --debug (-l) <level>              changes the log level (-1..4, default: 1)\n"
		);
	exit_scepclient(message);
}

/**
 * @brief main of scepclient
 *
 * @param argc number of arguments
 * @param argv pointer to the argument values
 */
int main(int argc, char **argv)
{
	/* external values */
	extern char * optarg;
	extern int optind;

	/* type of input and output files */
	typedef enum {
		PKCS1      =  0x01,
		PKCS10     =  0x02,
		PKCS7      =  0x04,
		CERT_SELF  =  0x08,
		CERT       =  0x10,
		CACERT_ENC =  0x20,
		CACERT_SIG =  0x40,
	} scep_filetype_t;

	/* filetype to read from, defaults to "generate a key" */
	scep_filetype_t filetype_in = 0;

	/* filetype to write to, no default here */
	scep_filetype_t filetype_out = 0;

	/* input files */
	char *file_in_pkcs1      = DEFAULT_FILENAME_PKCS1;
	char *file_in_pkcs10     = DEFAULT_FILENAME_PKCS10;
	char *file_in_cert_self  = DEFAULT_FILENAME_CERT_SELF;
	char *file_in_cacert_enc = DEFAULT_FILENAME_CACERT_ENC;
	char *file_in_cacert_sig = DEFAULT_FILENAME_CACERT_SIG;

	/* output files */
	char *file_out_pkcs1     = DEFAULT_FILENAME_PKCS1;
	char *file_out_pkcs10    = DEFAULT_FILENAME_PKCS10;
	char *file_out_pkcs7     = DEFAULT_FILENAME_PKCS7;
	char *file_out_cert_self = DEFAULT_FILENAME_CERT_SELF;
	char *file_out_cert      = DEFAULT_FILENAME_CERT;
	char *file_out_ca_cert   = DEFAULT_FILENAME_CACERT_ENC;

	/* by default user certificate is requested */
	bool request_ca_certificate = FALSE;

	/* by default existing files are not overwritten */
	bool force = FALSE;

	/* length of RSA key in bits */
	u_int rsa_keylength = DEFAULT_RSA_KEY_LENGTH;

	/* validity of self-signed certificate */
	time_t validity  = DEFAULT_CERT_VALIDITY;
	time_t notBefore = 0;
	time_t notAfter  = 0;

	/* distinguished name for requested certificate, ASCII format */
	char *distinguishedName = NULL;
	char default_distinguished_name[BUF_LEN];

	/* challenge password */
	char challenge_password_buffer[MAX_PASSWORD_LENGTH];

	/* symmetric encryption algorithm used by pkcs7, default is DES */
	encryption_algorithm_t pkcs7_symmetric_cipher = ENCR_DES;
	size_t pkcs7_key_size = 0;

	/* digest algorithm used by pkcs7, default is MD5 */
	hash_algorithm_t pkcs7_digest_alg = HASH_MD5;

	/* signature algorithm used by pkcs10, default is MD5 */
	hash_algorithm_t pkcs10_signature_alg = HASH_MD5;

	/* URL of the SCEP-Server */
	char *scep_url = NULL;

	/* Name of CA to fetch CA certs for */
	char *ca_name = "CAIdentifier";

	/* http request method, default is GET */
	bool http_get_request = TRUE;

	/* poll interval time in manual mode in seconds */
	u_int poll_interval = DEFAULT_POLL_INTERVAL;

	/* maximum poll time */
	u_int max_poll_time = 0;

	err_t ugh = NULL;

	/* initialize library */
	if (!library_init(NULL, "scepclient"))
	{
		library_deinit();
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "scepclient", argv[0]))
	{
		fprintf(stderr, "integrity check of scepclient failed\n");
		library_deinit();
		exit(SS_RC_DAEMON_INTEGRITY);
	}

	/* initialize global variables */
	pkcs1             = chunk_empty;
	pkcs7             = chunk_empty;
	serialNumber      = chunk_empty;
	transID           = chunk_empty;
	fingerprint       = chunk_empty;
	encoding          = chunk_empty;
	pkcs10_encoding   = chunk_empty;
	issuerAndSubject  = chunk_empty;
	challengePassword = chunk_empty;
	getCertInitial    = chunk_empty;
	scep_response     = chunk_empty;
	subjectAltNames   = linked_list_create();
	options           = options_create();

	for (;;)
	{
		static const struct option long_opts[] = {
			/* name, has_arg, flag, val */
			{ "help", no_argument, NULL, 'h' },
			{ "version", no_argument, NULL, 'v' },
			{ "optionsfrom", required_argument, NULL, '+' },
			{ "quiet", no_argument, NULL, 'q' },
			{ "debug", required_argument, NULL, 'l' },
			{ "in", required_argument, NULL, 'i' },
			{ "out", required_argument, NULL, 'o' },
			{ "force", no_argument, NULL, 'f' },
			{ "httptimeout", required_argument, NULL, 'T' },
			{ "bind", required_argument, NULL, 'b' },
			{ "keylength", required_argument, NULL, 'k' },
			{ "dn", required_argument, NULL, 'd' },
			{ "days", required_argument, NULL, 'D' },
			{ "startdate", required_argument, NULL, 'S' },
			{ "enddate", required_argument, NULL, 'E' },
			{ "subjectAltName", required_argument, NULL, 's' },
			{ "password", required_argument, NULL, 'p' },
			{ "algorithm", required_argument, NULL, 'a' },
			{ "url", required_argument, NULL, 'u' },
			{ "caname", required_argument, NULL, 'c'},
			{ "method", required_argument, NULL, 'm' },
			{ "interval", required_argument, NULL, 't' },
			{ "maxpolltime", required_argument, NULL, 'x' },
			{ 0,0,0,0 }
		};

		/* parse next option */
		int c = getopt_long(argc, argv, "hv+:qi:o:fk:d:s:p:a:u:c:m:t:x:APRCMS", long_opts, NULL);

		switch (c)
		{
			case EOF:       /* end of flags */
				break;

			case 'h':       /* --help */
				usage(NULL);

			case 'v':       /* --version */
				version();

			case 'q':       /* --quiet */
				log_to_stderr = FALSE;
				continue;

			case 'l':		/* --debug <level> */
				default_loglevel = atoi(optarg);
				continue;

			case 'i':       /* --in <type> [= <filename>] */
			{
				char *filename = strstr(optarg, "=");

				if (filename)
				{
					/* replace '=' by '\0' */
					*filename = '\0';
					/* set pointer to start of filename */
					filename++;
				}
				if (strcaseeq("pkcs1", optarg))
				{
					filetype_in |= PKCS1;
					if (filename)
						file_in_pkcs1 = filename;
				}
				else if (strcaseeq("pkcs10", optarg))
				{
					filetype_in |= PKCS10;
					if (filename)
						file_in_pkcs10 = filename;
				}
				else if (strcaseeq("cacert-enc", optarg))
				{
					filetype_in |= CACERT_ENC;
					if (filename)
						file_in_cacert_enc = filename;
				}
				else if (strcaseeq("cacert-sig", optarg))
				{
					filetype_in |= CACERT_SIG;
					if (filename)
						file_in_cacert_sig = filename;
				}
				else if (strcaseeq("cert-self", optarg))
				{
					filetype_in |= CERT_SELF;
					if (filename)
						file_in_cert_self = filename;
				}
				else
				{
					usage("invalid --in file type");
				}
				continue;
			}

			case 'o':       /* --out <type> [= <filename>] */
			{
				char *filename = strstr(optarg, "=");

				if (filename)
				{
					/* replace '=' by '\0' */
					*filename = '\0';
					/* set pointer to start of filename */
					filename++;
				}
				if (strcaseeq("pkcs1", optarg))
				{
					filetype_out |= PKCS1;
					if (filename)
						file_out_pkcs1 = filename;
				}
				else if (strcaseeq("pkcs10", optarg))
				{
					filetype_out |= PKCS10;
					if (filename)
						file_out_pkcs10 = filename;
				}
				else if (strcaseeq("pkcs7", optarg))
				{
					filetype_out |= PKCS7;
					if (filename)
						file_out_pkcs7 = filename;
				}
				else if (strcaseeq("cert-self", optarg))
				{
					filetype_out |= CERT_SELF;
					if (filename)
						file_out_cert_self = filename;
				}
				else if (strcaseeq("cert", optarg))
				{
					filetype_out |= CERT;
					if (filename)
						file_out_cert = filename;
				}
				else if (strcaseeq("cacert", optarg))
				{
					request_ca_certificate = TRUE;
					if (filename)
						file_out_ca_cert = filename;
				}
				else
				{
					usage("invalid --out file type");
				}
				continue;
			}

			case 'f':       /* --force */
				force = TRUE;
				continue;

			case 'T':       /* --httptimeout */
				http_timeout = atoi(optarg);
				if (http_timeout <= 0)
				{
					usage("invalid httptimeout specified");
				}
				continue;

			case 'b':       /* --bind */
				http_bind = optarg;
				continue;

			case '+':       /* --optionsfrom <filename> */
				if (!options->from(options, optarg, &argc, &argv, optind))
				{
					exit_scepclient("optionsfrom failed");
				}
				continue;

			case 'k':        /* --keylength <length> */
			{
				div_t q;

				rsa_keylength = atoi(optarg);
				if (rsa_keylength == 0)
					usage("invalid keylength");

				/* check if key length is a multiple of 8 bits */
				q = div(rsa_keylength, 2*BITS_PER_BYTE);
				if (q.rem != 0)
				{
					exit_scepclient("keylength is not a multiple of %d bits!"
						, 2*BITS_PER_BYTE);
				}
				continue;
			}

			case 'D':       /* --days */
				if (optarg == NULL || !isdigit(optarg[0]))
				{
					usage("missing number of days");
				}
				else
				{
					char *endptr;
					long days = strtol(optarg, &endptr, 0);

					if (*endptr != '\0' || endptr == optarg
					|| days <= 0)
						usage("<days> must be a positive number");
					validity = 24*3600*days;
				}
				continue;

			case 'S':       /* --startdate */
				if (optarg == NULL || strlen(optarg) != 13 || optarg[12] != 'Z')
				{
					usage("date format must be YYMMDDHHMMSSZ");
				}
				else
				{
					chunk_t date = { optarg, 13 };
					notBefore = asn1_to_time(&date, ASN1_UTCTIME);
				}
				continue;

			case 'E':       /* --enddate */
				if (optarg == NULL || strlen(optarg) != 13 || optarg[12] != 'Z')
				{
					usage("date format must be YYMMDDHHMMSSZ");
				}
				else
				{
					chunk_t date = { optarg, 13 };
					notAfter = asn1_to_time(&date, ASN1_UTCTIME);
				}
				continue;

			case 'd':       /* --dn */
				if (distinguishedName)
				{
					usage("only one distinguished name allowed");
				}
				distinguishedName = optarg;
				continue;

			case 's':       /* --subjectAltName */
			{
				char *value = strstr(optarg, "=");

				if (value)
				{
					/* replace '=' by '\0' */
					*value = '\0';
					/* set pointer to start of value */
					value++;
				}

				if (strcaseeq("email", optarg) ||
					strcaseeq("dns", optarg) ||
					strcaseeq("ip", optarg))
				{
					subjectAltNames->insert_last(subjectAltNames,
								 identification_create_from_string(value));
					continue;
				}
				else
				{
					usage("invalid --subjectAltName type");
					continue;
				}
			}

			case 'p':       /* --password */
				if (challengePassword.len > 0)
				{
					usage("only one challenge password allowed");
				}
				if (strcaseeq("%prompt", optarg))
				{
					printf("Challenge password: ");
					if (fgets(challenge_password_buffer,
							sizeof(challenge_password_buffer) - 1, stdin))
					{
						challengePassword.ptr = challenge_password_buffer;
						/* discard the terminating '\n' from the input */
						challengePassword.len = strlen(challenge_password_buffer) - 1;
					}
					else
					{
						usage("challenge password could not be read");
					}
				}
				else
				{
					challengePassword.ptr = optarg;
					challengePassword.len = strlen(optarg);
				}
				continue;

			case 'u':       /* -- url */
				if (scep_url)
				{
					usage("only one URL argument allowed");
				}
				scep_url = optarg;
				continue;

			case 'c':       /* -- caname */
				ca_name = optarg;
				continue;

			case 'm':       /* --method */
				if (strcaseeq("get", optarg))
				{
					http_get_request = TRUE;
				}
				else if (strcaseeq("post", optarg))
				{
					http_get_request = FALSE;
				}
				else
				{
					usage("invalid http request method specified");
				}
				continue;

			case 't':       /* --interval */
				poll_interval = atoi(optarg);
				if (poll_interval <= 0)
				{
					usage("invalid interval specified");
				}
				continue;

			case 'x':       /* --maxpolltime */
				max_poll_time = atoi(optarg);
				continue;

			case 'a':       /*--algorithm [<type>=]algo */
			{
				const proposal_token_t *token;
				char *type = optarg;
				char *algo = strstr(optarg, "=");

				if (algo)
				{
					*algo = '\0';
					algo++;
				}
				else
				{
					type = "enc";
					algo = optarg;
				}

				if (strcaseeq("enc", type))
				{
					token = lib->proposal->get_token(lib->proposal, algo);
					if (token == NULL || token->type != ENCRYPTION_ALGORITHM)
					{
						usage("invalid algorithm specified");
					}
					pkcs7_symmetric_cipher = token->algorithm;
					pkcs7_key_size = token->keysize;
					if (encryption_algorithm_to_oid(token->algorithm,
								token->keysize) == OID_UNKNOWN)
					{
						usage("unsupported encryption algorithm specified");
					}
				}
				else if (strcaseeq("dgst", type) ||
						 strcaseeq("sig", type))
				{
					hash_algorithm_t hash;

					token = lib->proposal->get_token(lib->proposal, algo);
					if (token == NULL || token->type != INTEGRITY_ALGORITHM)
					{
						usage("invalid algorithm specified");
					}
					hash = hasher_algorithm_from_integrity(token->algorithm,
														   NULL);
					if (hash == (hash_algorithm_t)OID_UNKNOWN)
					{
						usage("invalid algorithm specified");
					}
					if (strcaseeq("dgst", type))
					{
						pkcs7_digest_alg = hash;
					}
					else
					{
						pkcs10_signature_alg = hash;
					}
				}
				else
				{
					usage("invalid --algorithm type");
				}
				continue;
			}
			default:
				usage("unknown option");
		}
		/* break from loop */
		break;
	}

	init_log("scepclient");

	/* load plugins, further infrastructure may need it */
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "scepclient.load", PLUGINS)))
	{
		exit_scepclient("plugin loading failed");
	}
	lib->plugins->status(lib->plugins, LEVEL_DIAG);

	if ((filetype_out == 0) && (!request_ca_certificate))
	{
		usage("--out filetype required");
	}
	if (request_ca_certificate && (filetype_out > 0 || filetype_in > 0))
	{
		usage("in CA certificate request, no other --in or --out option allowed");
	}

	/* check if url is given, if cert output defined */
	if (((filetype_out & CERT) || request_ca_certificate) && !scep_url)
	{
		usage("URL of SCEP server required");
	}

	/* check for sanity of --in/--out */
	if (!filetype_in && (filetype_in > filetype_out))
	{
		usage("cannot generate --out of given --in!");
	}

	/* get CA cert */
	if (request_ca_certificate)
	{
		char ca_path[PATH_MAX];
		container_t *container;
		pkcs7_t *pkcs7;

		if (!scep_http_request(scep_url, chunk_create(ca_name, strlen(ca_name)),
							   SCEP_GET_CA_CERT, http_get_request,
							   http_timeout, http_bind, &scep_response))
		{
			exit_scepclient("did not receive a valid scep response");
		}

		join_paths(ca_path, sizeof(ca_path), CA_CERT_PATH, file_out_ca_cert);

		pkcs7 = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS7,
								BUILD_BLOB_ASN1_DER, scep_response, BUILD_END);

		if (!pkcs7)
		{	/* no PKCS#7 encoded CA+RA certificates, assume simple CA cert */

			DBG1(DBG_APP, "unable to parse PKCS#7, assuming plain CA cert");
			if (!chunk_write(scep_response, ca_path, 0022, force))
			{
				exit_scepclient("could not write ca cert file '%s': %s",
								ca_path, strerror(errno));
			}
		}
		else
		{
			enumerator_t *enumerator;
			certificate_t *cert;
			int ra_certs = 0, ca_certs = 0;
			int ra_index = 1, ca_index = 1;

			enumerator = pkcs7->create_cert_enumerator(pkcs7);
			while (enumerator->enumerate(enumerator, &cert))
			{
				x509_t *x509 = (x509_t*)cert;
				if (x509->get_flags(x509) & X509_CA)
				{
					ca_certs++;
				}
				else
				{
					ra_certs++;
				}
			}
			enumerator->destroy(enumerator);

			enumerator = pkcs7->create_cert_enumerator(pkcs7);
			while (enumerator->enumerate(enumerator, &cert))
			{
				x509_t *x509 = (x509_t*)cert;
				bool ca_cert = x509->get_flags(x509) & X509_CA;
				char cert_path[PATH_MAX], *path = ca_path;

				if (ca_cert && ca_certs > 1)
				{
					add_path_suffix(cert_path, sizeof(cert_path), ca_path,
									"-%.1d", ca_index++);
					path = cert_path;
				}
				else if (!ca_cert)
				{	/* use CA name as base for RA certs */
					if (ra_certs > 1)
					{
						add_path_suffix(cert_path, sizeof(cert_path), ca_path,
										"-ra-%.1d", ra_index++);
					}
					else
					{
						add_path_suffix(cert_path, sizeof(cert_path), ca_path,
										"-ra");
					}
					path = cert_path;
				}

				if (!cert->get_encoding(cert, CERT_ASN1_DER, &encoding) ||
					!chunk_write(encoding, path, 0022, force))
				{
					exit_scepclient("could not write cert file '%s': %s",
									path, strerror(errno));
				}
				chunk_free(&encoding);
			}
			enumerator->destroy(enumerator);
			container = &pkcs7->container;
			container->destroy(container);
		}
		exit_scepclient(NULL); /* no further output required */
	}

	creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &creds->set);

	/*
	 * input of PKCS#1 file
	 */
	if (filetype_in & PKCS1)    /* load an RSA key pair from file */
	{
		char path[PATH_MAX];

		join_paths(path, sizeof(path), PRIVATE_KEY_PATH, file_in_pkcs1);

		private_key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
										 BUILD_FROM_FILE, path, BUILD_END);
	}
	else                                /* generate an RSA key pair */
	{
		private_key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
										 BUILD_KEY_SIZE, rsa_keylength,
										 BUILD_END);
	}
	if (private_key == NULL)
	{
		exit_scepclient("no RSA private key available");
	}
	creds->add_key(creds, private_key->get_ref(private_key));
	public_key = private_key->get_public_key(private_key);

	/* check for minimum key length */
	if (private_key->get_keysize(private_key) < RSA_MIN_OCTETS / BITS_PER_BYTE)
	{
		exit_scepclient("length of RSA key has to be at least %d bits",
						RSA_MIN_OCTETS * BITS_PER_BYTE);
	}

	/*
	 * input of PKCS#10 file
	 */
	if (filetype_in & PKCS10)
	{
		char path[PATH_MAX];

		join_paths(path, sizeof(path), REQ_PATH, file_in_pkcs10);

		pkcs10_req = lib->creds->create(lib->creds, CRED_CERTIFICATE,
										CERT_PKCS10_REQUEST, BUILD_FROM_FILE,
										path, BUILD_END);
		if (!pkcs10_req)
		{
			exit_scepclient("could not read certificate request '%s'", path);
		}
		subject = pkcs10_req->get_subject(pkcs10_req);
		subject = subject->clone(subject);
	}
	else
	{
		if (distinguishedName == NULL)
		{
			int n = sprintf(default_distinguished_name, DEFAULT_DN);

			/* set the common name to the hostname */
			if (gethostname(default_distinguished_name + n, BUF_LEN - n) ||
				strlen(default_distinguished_name) == n)
			{
				exit_scepclient("no hostname defined, use "
								"--dn <distinguished name> option");
			}
			distinguishedName = default_distinguished_name;
		}

		DBG2(DBG_APP, "dn: '%s'", distinguishedName);
		subject = identification_create_from_string(distinguishedName);
		if (subject->get_type(subject) != ID_DER_ASN1_DN)
		{
			exit_scepclient("parsing of distinguished name failed");
		}

		DBG2(DBG_APP, "building pkcs10 object:");
		pkcs10_req = lib->creds->create(lib->creds, CRED_CERTIFICATE,
										CERT_PKCS10_REQUEST,
										BUILD_SIGNING_KEY, private_key,
										BUILD_SUBJECT, subject,
										BUILD_SUBJECT_ALTNAMES, subjectAltNames,
										BUILD_CHALLENGE_PWD, challengePassword,
										BUILD_DIGEST_ALG, pkcs10_signature_alg,
										BUILD_END);
		if (!pkcs10_req)
		{
			exit_scepclient("generating pkcs10 request failed");
		}
	}
	pkcs10_req->get_encoding(pkcs10_req, CERT_ASN1_DER, &pkcs10_encoding);
	fingerprint = scep_generate_pkcs10_fingerprint(pkcs10_encoding);
	DBG1(DBG_APP, "  fingerprint:    %s", fingerprint.ptr);

	/*
	 * output of PKCS#10 file
	 */
	if (filetype_out & PKCS10)
	{
		char path[PATH_MAX];

		join_paths(path, sizeof(path), REQ_PATH, file_out_pkcs10);

		if (!chunk_write(pkcs10_encoding, path, 0022, force))
		{
			exit_scepclient("could not write pkcs10 file '%s': %s",
							path, strerror(errno));
		}
		filetype_out &= ~PKCS10;   /* delete PKCS10 flag */
	}

	if (!filetype_out)
	{
		exit_scepclient(NULL); /* no further output required */
	}

	/*
	 * output of PKCS#1 file
	 */
	if (filetype_out & PKCS1)
	{
		char path[PATH_MAX];

		join_paths(path, sizeof(path), PRIVATE_KEY_PATH, file_out_pkcs1);

		DBG2(DBG_APP, "building pkcs1 object:");
		if (!private_key->get_encoding(private_key, PRIVKEY_ASN1_DER, &pkcs1) ||
			!chunk_write(pkcs1, path, 0066, force))
		{
			exit_scepclient("could not write pkcs1 file '%s': %s",
							path, strerror(errno));
		}
		filetype_out &= ~PKCS1;   /* delete PKCS1 flag */
	}

	if (!filetype_out)
	{
		exit_scepclient(NULL); /* no further output required */
	}

	scep_generate_transaction_id(public_key, &transID, &serialNumber);
	DBG1(DBG_APP, "  transaction ID: %.*s", (int)transID.len, transID.ptr);

	/*
	 * read or generate self-signed X.509 certificate
	 */
	if (filetype_in & CERT_SELF)
	{
		char path[PATH_MAX];

		join_paths(path, sizeof(path), HOST_CERT_PATH, file_in_cert_self);

		x509_signer = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
										 BUILD_FROM_FILE, path, BUILD_END);
		if (!x509_signer)
		{
			exit_scepclient("could not read certificate file '%s'", path);
		}
	}
	else
	{
		notBefore = notBefore ? notBefore : time(NULL);
		notAfter  = notAfter  ? notAfter  : (notBefore + validity);
		x509_signer = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
										 BUILD_SIGNING_KEY, private_key,
										 BUILD_PUBLIC_KEY, public_key,
										 BUILD_SUBJECT, subject,
										 BUILD_NOT_BEFORE_TIME, notBefore,
										 BUILD_NOT_AFTER_TIME, notAfter,
										 BUILD_SERIAL, serialNumber,
										 BUILD_SUBJECT_ALTNAMES, subjectAltNames,
										 BUILD_END);
		if (!x509_signer)
		{
			exit_scepclient("generating certificate failed");
		}
	}
	creds->add_cert(creds, TRUE, x509_signer->get_ref(x509_signer));

	/*
	 * output of self-signed X.509 certificate file
	 */
	if (filetype_out & CERT_SELF)
	{
		char path[PATH_MAX];

		join_paths(path, sizeof(path), HOST_CERT_PATH, file_out_cert_self);

		if (!x509_signer->get_encoding(x509_signer, CERT_ASN1_DER, &encoding))
		{
			exit_scepclient("encoding certificate failed");
		}
		if (!chunk_write(encoding, path, 0022, force))
		{
			exit_scepclient("could not write self-signed cert file '%s': %s",
							path, strerror(errno));
		}
		chunk_free(&encoding);
		filetype_out &= ~CERT_SELF;   /* delete CERT_SELF flag */
	}

	if (!filetype_out)
	{
		exit_scepclient(NULL); /* no further output required */
	}

	/*
	 * load ca encryption certificate
	 */
	{
		char path[PATH_MAX];

		join_paths(path, sizeof(path), CA_CERT_PATH, file_in_cacert_enc);

		x509_ca_enc = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
										 BUILD_FROM_FILE, path, BUILD_END);
		if (!x509_ca_enc)
		{
			exit_scepclient("could not load encryption cacert file '%s'", path);
		}
	}

	/*
	 * input of PKCS#7 file
	 */
	if (filetype_in & PKCS7)
	{
		/* user wants to load a pkcs7 encrypted request
		 * operation is not yet supported!
		 * would require additional parsing of transaction-id

		   pkcs7 = pkcs7_read_from_file(file_in_pkcs7);

		 */
	}
	else
	{
		DBG2(DBG_APP, "building pkcs7 request");
		pkcs7 = scep_build_request(pkcs10_encoding,
								   transID, SCEP_PKCSReq_MSG, x509_ca_enc,
								   pkcs7_symmetric_cipher, pkcs7_key_size,
								   x509_signer, pkcs7_digest_alg, private_key);
		if (!pkcs7.ptr)
		{
			exit_scepclient("failed to build pkcs7 request");
		}
	}

	/*
	 * output pkcs7 encrypted and signed certificate request
	 */
	if (filetype_out & PKCS7)
	{
		char path[PATH_MAX];

		join_paths(path, sizeof(path), REQ_PATH, file_out_pkcs7);

		if (!chunk_write(pkcs7, path, 0022, force))
		{
			exit_scepclient("could not write pkcs7 file '%s': %s",
							path, strerror(errno));
		}
		filetype_out &= ~PKCS7;   /* delete PKCS7 flag */
	}

	if (!filetype_out)
	{
		exit_scepclient(NULL); /* no further output required */
	}

	/*
	 * output certificate fetch from SCEP server
	 */
	if (filetype_out & CERT)
	{
		bool stored = FALSE;
		certificate_t *cert;
		enumerator_t  *enumerator;
		char path[PATH_MAX];
		time_t poll_start = 0;
		pkcs7_t *p7;
		container_t *container = NULL;
		chunk_t chunk;
		scep_attributes_t attrs = empty_scep_attributes;

		join_paths(path, sizeof(path), CA_CERT_PATH, file_in_cacert_sig);

		x509_ca_sig = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
										 BUILD_FROM_FILE, path, BUILD_END);
		if (!x509_ca_sig)
		{
			exit_scepclient("could not load signature cacert file '%s'", path);
		}

		creds->add_cert(creds, TRUE, x509_ca_sig->get_ref(x509_ca_sig));

		if (!scep_http_request(scep_url, pkcs7, SCEP_PKI_OPERATION,
					http_get_request, http_timeout, http_bind, &scep_response))
		{
			exit_scepclient("did not receive a valid scep response");
		}
		ugh = scep_parse_response(scep_response, transID, &container, &attrs);
		if (ugh != NULL)
		{
			exit_scepclient(ugh);
		}

		/* in case of manual mode, we are going into a polling loop */
		if (attrs.pkiStatus == SCEP_PENDING)
		{
			identification_t *issuer = x509_ca_sig->get_subject(x509_ca_sig);

			DBG1(DBG_APP, "  scep request pending, polling every %d seconds",
				 poll_interval);
			poll_start = time_monotonic(NULL);
			issuerAndSubject = asn1_wrap(ASN1_SEQUENCE, "cc",
									issuer->get_encoding(issuer),
									subject->get_encoding(subject));
		}
		while (attrs.pkiStatus == SCEP_PENDING)
		{
			if (max_poll_time > 0 &&
				(time_monotonic(NULL) - poll_start >= max_poll_time))
			{
				exit_scepclient("maximum poll time reached: %d seconds"
							   , max_poll_time);
			}
			DBG2(DBG_APP, "going to sleep for %d seconds", poll_interval);
			sleep(poll_interval);
			free(scep_response.ptr);
			container->destroy(container);

			DBG2(DBG_APP, "fingerprint:    %.*s",
				 (int)fingerprint.len, fingerprint.ptr);
			DBG2(DBG_APP, "transaction ID: %.*s",
				 (int)transID.len, transID.ptr);

			chunk_free(&getCertInitial);
			getCertInitial = scep_build_request(issuerAndSubject,
								transID, SCEP_GetCertInitial_MSG, x509_ca_enc,
								pkcs7_symmetric_cipher, pkcs7_key_size,
								x509_signer, pkcs7_digest_alg, private_key);
			if (!getCertInitial.ptr)
			{
				exit_scepclient("failed to build scep request");
			}
			if (!scep_http_request(scep_url, getCertInitial, SCEP_PKI_OPERATION,
					http_get_request, http_timeout, http_bind, &scep_response))
			{
				exit_scepclient("did not receive a valid scep response");
			}
			ugh = scep_parse_response(scep_response, transID, &container, &attrs);
			if (ugh != NULL)
			{
				exit_scepclient(ugh);
			}
		}

		if (attrs.pkiStatus != SCEP_SUCCESS)
		{
			container->destroy(container);
			exit_scepclient("reply status is not 'SUCCESS'");
		}

		if (!container->get_data(container, &chunk))
		{
			container->destroy(container);
			exit_scepclient("extracting signed-data failed");
		}
		container->destroy(container);

		/* decrypt enveloped-data container */
		container = lib->creds->create(lib->creds,
									   CRED_CONTAINER, CONTAINER_PKCS7,
									   BUILD_BLOB_ASN1_DER, chunk,
									   BUILD_END);
		free(chunk.ptr);
		if (!container)
		{
			exit_scepclient("could not decrypt envelopedData");
		}

		if (!container->get_data(container, &chunk))
		{
			container->destroy(container);
			exit_scepclient("extracting encrypted-data failed");
		}
		container->destroy(container);

		/* parse signed-data container */
		container = lib->creds->create(lib->creds,
									   CRED_CONTAINER, CONTAINER_PKCS7,
									   BUILD_BLOB_ASN1_DER, chunk,
									   BUILD_END);
		free(chunk.ptr);
		if (!container)
		{
			exit_scepclient("could not parse singed-data");
		}
		/* no need to verify the signed-data container, the signature does NOT
		 * cover the contained certificates */

		/* store the end entity certificate */
		join_paths(path, sizeof(path), HOST_CERT_PATH, file_out_cert);

		p7 = (pkcs7_t*)container;
		enumerator = p7->create_cert_enumerator(p7);
		while (enumerator->enumerate(enumerator, &cert))
		{
			x509_t *x509 = (x509_t*)cert;

			if (!(x509->get_flags(x509) & X509_CA))
			{
				if (stored)
				{
					exit_scepclient("multiple certs received, only first stored");
				}
				if (!cert->get_encoding(cert, CERT_ASN1_DER, &encoding) ||
					!chunk_write(encoding, path, 0022, force))
				{
					exit_scepclient("could not write cert file '%s': %s",
									path, strerror(errno));
				}
				chunk_free(&encoding);
				stored = TRUE;
			}
		}
		enumerator->destroy(enumerator);
		container->destroy(container);
		chunk_free(&attrs.transID);
		chunk_free(&attrs.senderNonce);
		chunk_free(&attrs.recipientNonce);

		filetype_out &= ~CERT;   /* delete CERT flag */
	}

	exit_scepclient(NULL);
	return -1; /* should never be reached */
}
