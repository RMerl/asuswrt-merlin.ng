/*
 * Copyright (C) 2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include <errno.h>

#include "pki.h"

#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>

/**
 * Calculate the keyid of a key/certificate
 */
static int keyid()
{
	credential_type_t type = CRED_PRIVATE_KEY;
	int subtype = KEY_RSA;
	certificate_t *cert;
	private_key_t *private;
	public_key_t *public;
	char *file = NULL;
	void *cred;
	chunk_t id;
	char *arg;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 't':
				if (streq(arg, "rsa-priv"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_RSA;
				}
				else if (streq(arg, "ecdsa-priv"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_ECDSA;
				}
				else if (streq(arg, "pub"))
				{
					type = CRED_PUBLIC_KEY;
					subtype = KEY_ANY;
				}
				else if (streq(arg, "pkcs10"))
				{
					type = CRED_CERTIFICATE;
					subtype = CERT_PKCS10_REQUEST;
				}
				else if (streq(arg, "x509"))
				{
					type = CRED_CERTIFICATE;
					subtype = CERT_X509;
				}
				else
				{
					return command_usage( "invalid input type");
				}
				continue;
			case 'i':
				file = arg;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --keyid option");
		}
		break;
	}
	if (file)
	{
		cred = lib->creds->create(lib->creds, type, subtype,
								  BUILD_FROM_FILE, file, BUILD_END);
	}
	else
	{
		chunk_t chunk;

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

	if (type == CRED_PRIVATE_KEY)
	{
		private = cred;
		if (private->get_fingerprint(private, KEYID_PUBKEY_SHA1, &id))
		{
			printf("subjectKeyIdentifier:      %#B\n", &id);
		}
		if (private->get_fingerprint(private, KEYID_PUBKEY_INFO_SHA1, &id))
		{
			printf("subjectPublicKeyInfo hash: %#B\n", &id);
		}
		private->destroy(private);
	}
	else if (type == CRED_PUBLIC_KEY)
	{
		public = cred;
		if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &id))
		{
			printf("subjectKeyIdentifier:      %#B\n", &id);
		}
		if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &id))
		{
			printf("subjectPublicKeyInfo hash: %#B\n", &id);
		}
		public->destroy(public);
	}
	else
	{
		cert = cred;
		public = cert->get_public_key(cert);
		if (!public)
		{
			fprintf(stderr, "extracting public key from certificate failed");
			return 1;
		}
		if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &id))
		{
			printf("subjectKeyIdentifier:      %#B\n", &id);
		}
		if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &id))
		{
			printf("subjectPublicKeyInfo hash: %#B\n", &id);
		}
		public->destroy(public);
		cert->destroy(cert);
	}
	return 0;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t)
		{ keyid, 'k', "keyid",
		"calculate key identifiers of a key/certificate",
		{"[--in file] [--type rsa-priv|ecdsa-priv|pub|pkcs10|x509]"},
		{
			{"help",	'h', 0, "show usage information"},
			{"in",		'i', 1, "input file, default: stdin"},
			{"type",	't', 1, "type of key, default: rsa-priv"},
		}
	});
}
