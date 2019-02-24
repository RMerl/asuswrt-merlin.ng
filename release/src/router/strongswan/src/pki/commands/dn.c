/*
 * Copyright (C) 2015 Tobias Brunner
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

#include <errno.h>

/**
 * Extract subject DN
 */
static int dn()
{
	identification_t *id;
	certificate_t *cert;
	chunk_t chunk;
	enum {
		FORMAT_CONFIG,
		FORMAT_HEX,
		FORMAT_BASE64,
		FORMAT_BINARY,
	} format = FORMAT_CONFIG;
	char *arg, *file = NULL, *fmt;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
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
				else if (!streq(arg, "config"))
				{
					return command_usage( "invalid output format");
				}
				continue;
			case 'i':
				file = arg;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --print option");
		}
		break;
	}
	if (file)
	{
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
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
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_BLOB, chunk, BUILD_END);
		free(chunk.ptr);
	}
	if (!cert)
	{
		fprintf(stderr, "parsing input failed\n");
		return 1;
	}
	id = cert->get_subject(cert);
	if (!id)
	{
		fprintf(stderr, "failed to get certificate's subject DN\n");
		cert->destroy(cert);
		return 1;
	}
	fmt = "%.*s\n";
	switch (format)
	{
		case FORMAT_CONFIG:
			fmt = "\"asn1dn:#%.*s\"\n";
			/* fall-through */
		case FORMAT_HEX:
			chunk = chunk_to_hex(id->get_encoding(id), NULL, FALSE);
			printf(fmt, (int)chunk.len, chunk.ptr);
			chunk_free(&chunk);
			break;
		case FORMAT_BASE64:
			chunk = chunk_to_base64(id->get_encoding(id), NULL);
			printf(fmt, (int)chunk.len, chunk.ptr);
			chunk_free(&chunk);
			break;
		case FORMAT_BINARY:
			chunk = id->get_encoding(id);
			if (fwrite(chunk.ptr, chunk.len, 1, stdout) != 1)
			{
				fprintf(stderr, "writing subject DN failed\n");
			}
			break;
	}
	cert->destroy(cert);
	return 0;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t)
		{ dn, 'd', "dn",
		"extract the subject DN of an X.509 certificate",
		{"[--in file] [--format config|hex|base64|bin]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"in",			'i', 1, "input file, default: stdin"},
			{"format",		'f', 1, "output format, default: config"},
		}
	});
}
