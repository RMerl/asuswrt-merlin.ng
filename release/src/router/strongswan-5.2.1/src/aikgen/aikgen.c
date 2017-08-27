/*
 * Copyright (C) 2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (c) 2008 Hal Finney
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <library.h>
#include <utils/debug.h>
#include <utils/optionsfrom.h>
#include <credentials/certificates/x509.h>
#include <credentials/keys/public_key.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>

#include <trousers/tss.h>
#include <trousers/trousers.h>

#include <syslog.h>
#include <getopt.h>
#include <errno.h>

/* default directory where AIK keys are stored */
#define AIK_DIR							IPSEC_CONFDIR "/pts/"

/* default name of AIK private key blob */
#define DEFAULT_FILENAME_AIKBLOB		AIK_DIR "aikBlob.bin"

/* default name of AIK private key blob */
#define DEFAULT_FILENAME_AIKPUBKEY		AIK_DIR "aikPub.der"

/* size in bytes of a TSS AIK public key blob */
#define AIK_PUBKEY_BLOB_SIZE			284

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

/* TPM context */
TSS_HCONTEXT  hContext;

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
 * @param status 0 = OK, 1 = general discomfort
 */
static void exit_aikgen(err_t message, ...)
{
	int status = 0;

	DESTROY_IF(cacert);
	DESTROY_IF(ca_pubkey);
	free(ca_modulus.ptr);
	free(aik_pubkey.ptr);
	free(aik_keyid.ptr);
	options->destroy(options);

	/* clean up TPM context */
	if (hContext)
	{
		Tspi_Context_FreeMemory(hContext, NULL);
		Tspi_Context_Close(hContext);
	}

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
		"       aikgen --help\n"
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
	chunk_t aik_pubkey_blob;
	chunk_t aik_modulus;
	chunk_t aik_exponent;

	/* TPM variables */
	TSS_RESULT   result;
	TSS_HTPM     hTPM;
	TSS_HKEY     hSRK;
	TSS_HKEY     hPCAKey;
	TSS_HPOLICY  hSrkPolicy;
	TSS_HPOLICY  hTPMPolicy;
	TSS_HKEY     hIdentKey;
	TSS_UUID     SRK_UUID = TSS_UUID_SRK;
	BYTE         secret[] = TSS_WELL_KNOWN_SECRET;
	BYTE        *IdentityReq;
	UINT32       IdentityReqLen;
	BYTE        *blob;
	UINT32       blobLen;

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
		exit_aikgen("ca public key must be RSA 2048 but is %N %d",
					 key_type_names, ca_pubkey->get_type(ca_pubkey),
					 ca_pubkey->get_keysize(ca_pubkey));
	}
	if (!ca_pubkey->get_encoding(ca_pubkey, PUBKEY_RSA_MODULUS, &ca_modulus))
	{
		exit_aikgen("could not extract RSA modulus from ca public key");
	}

	/* initialize TSS context and connect to it */
	result = Tspi_Context_Create(&hContext);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_Context_Create", result);
	}
	result = Tspi_Context_Connect(hContext, NULL);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_Context_Connect", result);
	}

	/* get SRK plus SRK policy and set SRK secret */
	result = Tspi_Context_LoadKeyByUUID(hContext, TSS_PS_TYPE_SYSTEM,
										SRK_UUID, &hSRK);
 	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_Context_LoadKeyByUUID for SRK", result);
	}
	result = Tspi_GetPolicyObject(hSRK, TSS_POLICY_USAGE, &hSrkPolicy);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_GetPolicyObject for SRK", result);
	}
	result = Tspi_Policy_SetSecret(hSrkPolicy, TSS_SECRET_MODE_SHA1, 20, secret);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_Policy_SetSecret for SRK", result);
	}

	/* get TPM plus TPM policy and set TPM secret */
	result = Tspi_Context_GetTpmObject (hContext, &hTPM);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_Context_GetTpmObject", result);
	}
	result = Tspi_GetPolicyObject(hTPM, TSS_POLICY_USAGE, &hTPMPolicy);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_GetPolicyObject for TPM", result);
	}
	result = Tspi_Policy_SetSecret(hTPMPolicy, TSS_SECRET_MODE_SHA1, 20, secret);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_Policy_SetSecret for TPM", result);
	}

	/* create context for a 2048 bit AIK */
	result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_RSAKEY,
					TSS_KEY_TYPE_IDENTITY | TSS_KEY_SIZE_2048 |
					TSS_KEY_VOLATILE | TSS_KEY_NOT_MIGRATABLE, &hIdentKey);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_Context_CreateObject for key", result);
	}

	/* create context for the Privacy CA public key and assign modulus */
	result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_RSAKEY,
					TSS_KEY_TYPE_LEGACY|TSS_KEY_SIZE_2048, &hPCAKey);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_Context_CreateObject for PCA", result);
	}
	result = Tspi_SetAttribData (hPCAKey, TSS_TSPATTRIB_RSAKEY_INFO,
					TSS_TSPATTRIB_KEYINFO_RSA_MODULUS, ca_modulus.len,
					ca_modulus.ptr);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_SetAttribData for PCA modulus", result);
	}
	result = Tspi_SetAttribUint32(hPCAKey, TSS_TSPATTRIB_KEY_INFO,
					TSS_TSPATTRIB_KEYINFO_ENCSCHEME, TSS_ES_RSAESPKCSV15);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_SetAttribUint32 for PCA "
					"encryption scheme", result);
	}

	/* generate AIK */
	DBG1(DBG_LIB, "Generating identity key...");
	result = Tspi_TPM_CollateIdentityRequest(hTPM, hSRK, hPCAKey, 0, NULL,
					hIdentKey, TSS_ALG_AES,	&IdentityReqLen, &IdentityReq);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_TPM_CollateIdentityRequest", result);
	}
	identity_req = chunk_create(IdentityReq, IdentityReqLen);
	DBG3(DBG_LIB, "Identity Request: %B", &identity_req);

	/* optionally output identity request encrypted with ca public key */
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

	/* load identity key */
	result = Tspi_Key_LoadKey (hIdentKey, hSRK);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_Key_LoadKey for AIK\n", result);
	}

	/* output AIK private key in TSS blob format */
	result = Tspi_GetAttribData (hIdentKey, TSS_TSPATTRIB_KEY_BLOB,
					TSS_TSPATTRIB_KEYBLOB_BLOB, &blobLen, &blob);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_GetAttribData for private key blob",
					 result);
	}
	aik_blob = chunk_create(blob, blobLen);
	DBG3(DBG_LIB, "AIK private key blob: %B", &aik_blob);

	if (!chunk_write(aik_blob, aikblob_filename, 0022, force))
	{
		exit_aikgen("could not write AIK blob file '%s': %s",
					 aikblob_filename, strerror(errno));
	}
	DBG1(DBG_LIB, "AIK private key blob written to '%s' (%u bytes)",
				   aikblob_filename, aik_blob.len);

	/* output AIK Public Key in TSS blob format */
	result = Tspi_GetAttribData (hIdentKey, TSS_TSPATTRIB_KEY_BLOB,
					TSS_TSPATTRIB_KEYBLOB_PUBLIC_KEY, &blobLen, &blob);
	if (result != TSS_SUCCESS)
	{
		exit_aikgen("tss 0x%x on Tspi_GetAttribData for public key blob",
					 result);
	}
	aik_pubkey_blob = chunk_create(blob, blobLen);
	DBG3(DBG_LIB, "AIK public key blob: %B", &aik_pubkey_blob);

	/* create a trusted AIK public key */
	if (aik_pubkey_blob.len != AIK_PUBKEY_BLOB_SIZE)
	{
		exit_aikgen("AIK public key is not in TSS blob format");
	}
	aik_modulus = chunk_skip(aik_pubkey_blob, AIK_PUBKEY_BLOB_SIZE - 256);
	aik_exponent = chunk_from_chars(0x01, 0x00, 0x01);

	/* output subjectPublicKeyInfo encoding of AIK public key */
	if (!lib->encoding->encode(lib->encoding, PUBKEY_SPKI_ASN1_DER, NULL,
					&aik_pubkey, CRED_PART_RSA_MODULUS, aik_modulus,
					CRED_PART_RSA_PUB_EXP, aik_exponent, CRED_PART_END))
	{
		exit_aikgen("subjectPublicKeyInfo encoding of AIK key failed");
	}
	if (!chunk_write(aik_pubkey, aikpubkey_filename, 0022, force))
	{
		exit_aikgen("could not write AIK public key file '%s': %s",
					 aikpubkey_filename, strerror(errno));
	}
	DBG1(DBG_LIB, "AIK public key written to '%s' (%u bytes)",
				   aikpubkey_filename, aik_pubkey.len);

	/* display AIK keyid derived from subjectPublicKeyInfo encoding */
	if (!lib->encoding->encode(lib->encoding, KEYID_PUBKEY_INFO_SHA1, NULL,
					&aik_keyid, CRED_PART_RSA_MODULUS, aik_modulus,
					CRED_PART_RSA_PUB_EXP, aik_exponent, CRED_PART_END))
	{
		exit_aikgen("computation of AIK keyid failed");
	}
	DBG1(DBG_LIB, "AIK keyid: %#B", &aik_keyid);

	exit_aikgen(NULL);
	return -1; /* should never be reached */
}
