/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 *
 * Copyright (C) 2015-2016 Andreas Steffen
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

#include "pki.h"

#include <credentials/certificates/certificate.h>
#include <credentials/certificates/certificate_printer.h>

#include <errno.h>

/**
 * Print private key information
 */
static void print_key(private_key_t *key)
{
	public_key_t *public;
	chunk_t chunk;

	public = key->get_public_key(key);
	if (public)
	{
		printf("  privkey:   %N %d bits\n", key_type_names,
			   public->get_type(public), public->get_keysize(public));
		if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &chunk))
		{
			printf("  keyid:     %#B\n", &chunk);
		}
		if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &chunk))
		{
			printf("  subjkey:   %#B\n", &chunk);
		}
		public->destroy(public);
	}
	else
	{
		printf("extracting public from private key failed\n");
	}
}

/**
 * Print a credential in a human readable form
 */
static int print()
{
	credential_type_t type = CRED_CERTIFICATE;
	int subtype = CERT_X509;
	void *cred;
	char *arg, *file = NULL, *keyid = NULL;
	chunk_t chunk;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 't':
				if (streq(arg, "x509"))
				{
					type = CRED_CERTIFICATE;
					subtype = CERT_X509;
				}
				else if (streq(arg, "crl"))
				{
					type = CRED_CERTIFICATE;
					subtype = CERT_X509_CRL;
				}
				else if (streq(arg, "ac"))
				{
					type = CRED_CERTIFICATE;
					subtype = CERT_X509_AC;
				}
				else if (streq(arg, "pub"))
				{
					type = CRED_CERTIFICATE;
					subtype = CERT_TRUSTED_PUBKEY;
				}
				else if (streq(arg, "priv"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_ANY;
				}
				else if (streq(arg, "rsa") ||
						 streq(arg, "rsa-priv"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_RSA;
				}
				else if (streq(arg, "ecdsa") ||
						 streq(arg, "ecdsa-priv"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_ECDSA;
				}
				else if (streq(arg, "ed25519") ||
						 streq(arg, "ed25519-priv"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_ED25519;
				}
				else if (streq(arg, "bliss") ||
						 streq(arg, "bliss-priv"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_BLISS;
				}
				else
				{
					return command_usage( "invalid input type");
				}
				continue;
			case 'i':
				file = arg;
				continue;
			case 'x':
				keyid = arg;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --print option");
		}
		break;
	}
	if (keyid)
	{
		chunk = chunk_from_hex(chunk_create(keyid, strlen(keyid)), NULL);
		cred = lib->creds->create(lib->creds, type, subtype,
								  BUILD_PKCS11_KEYID, chunk, BUILD_END);
		free(chunk.ptr);
	}
	else if (file)
	{
		cred = lib->creds->create(lib->creds, type, subtype,
								  BUILD_FROM_FILE, file, BUILD_END);
	}
	else
	{
		set_file_mode(stdin, CERT_ASN1_DER);
		if (!chunk_from_fd(0, &chunk))
		{
			fprintf(stderr, "reading input failed: %s\n", strerror(errno));
			return 1;
		}
		cred = lib->creds->create(lib->creds, type, subtype,
								  BUILD_BLOB, chunk, BUILD_END);
		free(chunk.ptr);
	}
	if (!cred)
	{
		fprintf(stderr, "parsing input failed\n");
		return 1;
	}

	if (type == CRED_CERTIFICATE)
	{
		certificate_t *cert = (certificate_t*)cred;
		certificate_printer_t *printer;

		printer = certificate_printer_create(stdout, TRUE, FALSE);
		printer->print(printer, cert, FALSE);
		printer->destroy(printer);
		cert->destroy(cert);
	}
	if (type == CRED_PRIVATE_KEY)
	{
		private_key_t *key = (private_key_t*)cred;

		print_key(key);
		key->destroy(key);
	}

	return 0;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t)
		{ print, 'a', "print",
		"print a credential in a human readable form",
		{"[--in file|--keyid hex] "
		 "[--type x509|crl|ac|pub|priv|rsa|ecdsa|ed25519|bliss]"},
		{
			{"help",	'h', 0, "show usage information"},
			{"in",		'i', 1, "input file, default: stdin"},
			{"keyid",	'x', 1, "smartcard or TPM object handle"},
			{"type",	't', 1, "type of credential, default: x509"},
		}
	});
}
