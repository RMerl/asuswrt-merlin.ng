/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2009 Andreas Steffen
 *
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

#include <time.h>
#include <errno.h>

#include "pki.h"

#include <collections/linked_list.h>
#include <credentials/certificates/certificate.h>

/**
 * Create a self-signed PKCS#10 certificate requesst.
 */
static int req()
{
	cred_encoding_type_t form = CERT_ASN1_DER;
	key_type_t type = KEY_RSA;
	hash_algorithm_t digest = HASH_SHA1;
	certificate_t *cert = NULL;
	private_key_t *private = NULL;
	char *file = NULL, *dn = NULL, *error = NULL;
	identification_t *id = NULL;
	linked_list_t *san;
	chunk_t encoding = chunk_empty;
	chunk_t challenge_password = chunk_empty;
	char *arg;

	san = linked_list_create();

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				goto usage;
			case 't':
				if (streq(arg, "rsa"))
				{
					type = KEY_RSA;
				}
				else if (streq(arg, "ecdsa"))
				{
					type = KEY_ECDSA;
				}
				else
				{
					error = "invalid input type";
					goto usage;
				}
				continue;
			case 'g':
				if (!enum_from_name(hash_algorithm_short_names, arg, &digest))
				{
					error = "invalid --digest type";
					goto usage;
				}
				continue;
			case 'i':
				file = arg;
				continue;
			case 'd':
				dn = arg;
				continue;
			case 'a':
				san->insert_last(san, identification_create_from_string(arg));
				continue;
			case 'p':
				challenge_password = chunk_create(arg, strlen(arg));
				continue;
			case 'f':
				if (!get_form(arg, &form, CRED_CERTIFICATE))
				{
					error = "invalid output format";
					goto usage;
				}
				continue;
			case EOF:
				break;
			default:
				error = "invalid --req option";
				goto usage;
		}
		break;
	}

	if (!dn)
	{
		error = "--dn is required";
		goto usage;
	}
	id = identification_create_from_string(dn);
	if (id->get_type(id) != ID_DER_ASN1_DN)
	{
		error = "supplied --dn is not a distinguished name";
		goto end;
	}
	if (file)
	{
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
									 BUILD_FROM_FILE, file, BUILD_END);
	}
	else
	{
		chunk_t chunk;

		set_file_mode(stdin, CERT_ASN1_DER);
		if (!chunk_from_fd(0, &chunk))
		{
			fprintf(stderr, "reading private key failed: %s\n", strerror(errno));
			error = "";
			goto end;
		}
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
									 BUILD_BLOB, chunk, BUILD_END);
		free(chunk.ptr);
	}
	if (!private)
	{
		error = "parsing private key failed";
		goto end;
	}
	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_PKCS10_REQUEST,
							  BUILD_SIGNING_KEY, private,
							  BUILD_SUBJECT, id,
							  BUILD_SUBJECT_ALTNAMES, san,
							  BUILD_CHALLENGE_PWD, challenge_password,
							  BUILD_DIGEST_ALG, digest,
							  BUILD_END);
	if (!cert)
	{
		error = "generating certificate request failed";
		goto end;
	}
	if (!cert->get_encoding(cert, form, &encoding))
	{
		error = "encoding certificate request failed";
		goto end;
	}
	set_file_mode(stdout, form);
	if (fwrite(encoding.ptr, encoding.len, 1, stdout) != 1)
	{
		error = "writing certificate request failed";
		goto end;
	}

end:
	DESTROY_IF(id);
	DESTROY_IF(cert);
	DESTROY_IF(private);
	san->destroy_offset(san, offsetof(identification_t, destroy));
	free(encoding.ptr);

	if (error)
	{
		fprintf(stderr, "%s\n", error);
		return 1;
	}
	return 0;

usage:
	san->destroy_offset(san, offsetof(identification_t, destroy));
	return command_usage(error);
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		req, 'r', "req",
		"create a PKCS#10 certificate request",
		{"  [--in file] [--type rsa|ecdsa] --dn distinguished-name",
		 "[--san subjectAltName]+ [--password challengePassword]",
		 "[--digest md5|sha1|sha224|sha256|sha384|sha512] [--outform der|pem]"},
		{
			{"help",	'h', 0, "show usage information"},
			{"in",		'i', 1, "private key input file, default: stdin"},
			{"type",	't', 1, "type of input key, default: rsa"},
			{"dn",		'd', 1, "subject distinguished name"},
			{"san",		'a', 1, "subjectAltName to include in cert request"},
			{"password",'p', 1, "challengePassword to include in cert request"},
			{"digest",	'g', 1, "digest for signature creation, default: sha1"},
			{"outform",	'f', 1, "encoding of generated request, default: der"},
		}
	});
}
