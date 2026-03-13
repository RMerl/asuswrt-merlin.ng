/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2017 Andreas Steffen
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

#include <errno.h>

#include "pki.h"

#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>

typedef enum {
		FORMAT_PRETTY,
		FORMAT_HEX,
		FORMAT_BASE64,
		FORMAT_BINARY,
} format_t;

/**
 * Print a single keyid in the requested format
 */
static bool print_id(chunk_t id, format_t format, char *desc)
{
	chunk_t chunk;

	switch (format)
	{
		case FORMAT_PRETTY:
			printf("%s:\n             %#B\n", desc, &id);
			break;
		case FORMAT_HEX:
			chunk = chunk_to_hex(id, NULL, FALSE);
			printf("%.*s\n", (int)chunk.len, chunk.ptr);
			chunk_free(&chunk);
			break;
		case FORMAT_BASE64:
			chunk = chunk_to_base64(id, NULL);
			printf("%.*s\n", (int)chunk.len, chunk.ptr);
			chunk_free(&chunk);
			break;
		case FORMAT_BINARY:
			if (fwrite(id.ptr, id.len, 1, stdout) != 1)
			{
				fprintf(stderr, "writing %s failed\n", desc);
				return FALSE;
			}
			break;
	}
	return TRUE;
}

/**
 * Calculate the keyid of a key/certificate
 */
static int keyid()
{
	credential_type_t type = CRED_PRIVATE_KEY;
	int subtype = KEY_ANY;
	certificate_t *cert;
	private_key_t *private;
	public_key_t *public;
	format_t format = FORMAT_PRETTY;
	enum {
		ID_TYPE_ALL,
		ID_TYPE_SPK,
		ID_TYPE_SPKI,
	} id_type = ID_TYPE_ALL;
	char *file = NULL, *keyid = NULL;
	void *cred;
	chunk_t id, spk = chunk_empty, spki = chunk_empty;
	char *arg;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 't':
				if (streq(arg, "rsa") ||
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
					return command_usage( "invalid input type");
				}
				continue;
			case 'I':
				if (streq(arg, "spk"))
				{
					id_type = ID_TYPE_SPK;
				}
				else if (streq(arg, "spki"))
				{
					id_type = ID_TYPE_SPKI;
				}
				else if (!streq(arg, "all"))
				{
					return command_usage( "invalid id type");
				}
				continue;
			case 'f':
				if (streq(arg, "hex"))
				{
					format = FORMAT_HEX;
				}
				else if (streq(arg, "base64"))
				{
					format = FORMAT_BASE64;
				}
				else if (streq(arg, "bin"))
				{
					format = FORMAT_BINARY;
				}
				else if (!streq(arg, "pretty"))
				{
					return command_usage( "invalid output format");
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
				return command_usage("invalid --keyid option");
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
			spk = chunk_clone(id);
		}
		if (private->get_fingerprint(private, KEYID_PUBKEY_INFO_SHA1, &id))
		{
			spki = chunk_clone(id);
		}
		private->destroy(private);
	}
	else if (type == CRED_PUBLIC_KEY)
	{
		public = cred;
		if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &id))
		{
			spk = chunk_clone(id);
		}
		if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &id))
		{
			spki = chunk_clone(id);
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
			spk = chunk_clone(id);
		}
		if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &id))
		{
			spki = chunk_clone(id);
		}
		public->destroy(public);
		cert->destroy(cert);
	}

	if (id_type == ID_TYPE_ALL || id_type == ID_TYPE_SPK)
	{
		if (!spk.len ||
			!print_id(spk, format, "subjkey (SHA-1 of subjectPublicKey)"))
		{
			return 1;
		}
	}
	if (id_type == ID_TYPE_ALL || id_type == ID_TYPE_SPKI)
	{
		if (!spki.len ||
			!print_id(spki, format, "keyid (SHA-1 of subjectPublicKeyInfo)"))
		{
			return 1;
		}
	}
	chunk_free(&spk);
	chunk_free(&spki);
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
		{"[--in file|--keyid hex] [--type priv|rsa|ecdsa|pub|pkcs10|x509]",
		 "[--id all|spk|spki] [--format pretty|hex|base64|bin]"},
		{
			{"help",	'h', 0, "show usage information"},
			{"in",		'i', 1, "input file, default: stdin"},
			{"keyid",	'x', 1, "smartcard or TPM private key object handle"},
			{"type",	't', 1, "type of key, default: priv"},
			{"id",		'I', 1, "type of identifier, default: all"},
			{"format",	'f', 1, "output format, default: pretty"},
		}
	});
}
