/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2015-2017 Andreas Steffen
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

#include <errno.h>

#include "pki.h"

#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>

/**
 * Extract a public key from a private key/certificate
 */
static int pub()
{
	cred_encoding_type_t form = PUBKEY_SPKI_ASN1_DER;
	credential_type_t type = CRED_PRIVATE_KEY;
	int subtype = KEY_ANY;
	certificate_t *cert;
	private_key_t *private;
	public_key_t *public;
	chunk_t encoding;
	char *file = NULL, *keyid = NULL;
	void *cred;
	char *arg;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 't':
				if (streq(arg, "rsa"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_RSA;
				}
				else if (streq(arg, "ecdsa"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_ECDSA;
				}
				else if (streq(arg, "bliss"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_BLISS;
				}
				else if (streq(arg, "priv"))
				{
					type = CRED_PRIVATE_KEY;
					subtype = KEY_ANY;
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
					return command_usage("invalid input type");
				}
				continue;
			case 'f':
				if (!get_form(arg, &form, CRED_PUBLIC_KEY))
				{
					return command_usage("invalid output format");
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
				return command_usage("invalid --pub option");
		}
		break;
	}
	if (file)
	{
		cred = lib->creds->create(lib->creds, type, subtype,
									 BUILD_FROM_FILE, file, BUILD_END);
	}
	else if (keyid)
	{
		chunk_t chunk;

		chunk = chunk_from_hex(chunk_create(keyid, strlen(keyid)), NULL);
		cred = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_ANY,
								  BUILD_PKCS11_KEYID, chunk, BUILD_END);
		free(chunk.ptr);
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

	if (type == CRED_PRIVATE_KEY)
	{
		private = cred;
		if (!private)
		{
			fprintf(stderr, "parsing private key failed\n");
			return 1;
		}
		public = private->get_public_key(private);
		private->destroy(private);
	}
	else if (type == CRED_PUBLIC_KEY)
	{
		public = cred;
		if (!public)
		{
			fprintf(stderr, "parsing public key failed\n");
			return 1;
		}
	}
	else
	{
		cert = cred;
		if (!cert)
		{
			fprintf(stderr, "parsing certificate failed\n");
			return 1;
		}
		public = cert->get_public_key(cert);
		cert->destroy(cert);
	}
	if (!public)
	{
		fprintf(stderr, "extracting public key failed\n");
		return 1;
	}
	if (!public->get_encoding(public, form, &encoding))
	{
		fprintf(stderr, "public key encoding failed\n");
		public->destroy(public);
		return 1;
	}
	public->destroy(public);
	set_file_mode(stdout, form);
	if (fwrite(encoding.ptr, encoding.len, 1, stdout) != 1)
	{
		fprintf(stderr, "writing public key failed\n");
		free(encoding.ptr);
		return 1;
	}
	free(encoding.ptr);
	return 0;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		pub, 'p', "pub",
		"extract the public key from a private key/certificate",
		{"[--in file|--keyid hex] [--type rsa|ecdsa|bliss|priv|pub|pkcs10|x509]",
		 "[--outform der|pem|dnskey|sshkey]"},
		{
			{"help",	'h', 0, "show usage information"},
			{"in",		'i', 1, "input file, default: stdin"},
			{"keyid",	'x', 1, "smartcard or TPM private key object handle"},
			{"type",	't', 1, "type of credential, default: priv"},
			{"outform",	'f', 1, "encoding of extracted public key, default: der"},
		}
	});
}
