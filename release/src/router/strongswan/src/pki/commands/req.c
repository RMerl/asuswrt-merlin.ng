/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2009-2022 Andreas Steffen
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

#include <time.h>
#include <errno.h>

#include "pki.h"

#include <collections/linked_list.h>
#include <credentials/certificates/certificate.h>
#include <credentials/certificates/pkcs10.h>

/**
 * Create a self-signed PKCS#10 certificate request.
 */
static int req()
{
	cred_encoding_type_t form = CERT_ASN1_DER;
	key_type_t type = KEY_ANY;
	hash_algorithm_t digest = HASH_UNKNOWN;
	signature_params_t *scheme = NULL;
	certificate_t *cert = NULL, *oldreq = NULL;
	pkcs10_t *pkcs10;
	private_key_t *private = NULL;
	char *file = NULL, *keyid = NULL, *dn = NULL, *error = NULL;
	char *oldreq_file = NULL;
	identification_t *id = NULL;
	linked_list_t *san;
	chunk_t encoding = chunk_empty;
	chunk_t challenge_password = chunk_empty;
	chunk_t cert_type_ext = chunk_empty;
	char *arg;
	bool pss = lib->settings->get_bool(lib->settings, "%s.rsa_pss", FALSE,
									   lib->ns);

	san = linked_list_create();

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':       /* --help */
				goto usage;
			case 't':       /* --type */
				if (streq(arg, "rsa"))
				{
					type = KEY_RSA;
				}
				else if (streq(arg, "ecdsa"))
				{
					type = KEY_ECDSA;
				}
				else if (streq(arg, "bliss"))
				{
					type = KEY_BLISS;
				}
				else if (streq(arg, "priv"))
				{
					type = KEY_ANY;
				}
				else
				{
					error = "invalid input type";
					goto usage;
				}
				continue;
			case 'g':       /* --digest */
				if (!enum_from_name(hash_algorithm_short_names, arg, &digest))
				{
					error = "invalid --digest type";
					goto usage;
				}
				continue;
			case 'R':       /* --rsa-padding */
				if (streq(arg, "pss"))
				{

					pss = TRUE;
				}
				else if (!streq(arg, "pkcs1"))
				{
					error = "invalid RSA padding";
					goto usage;
				}
				continue;
			case 'i':       /* --in */
				file = arg;
				continue;
			case 'd':       /* --dn */
				dn = arg;
				continue;
			case 'a':       /* --san */
				san->insert_last(san, identification_create_from_string(arg));
				continue;
			case 'P':       /* --profile */
				cert_type_ext = chunk_create(arg, strlen(arg));
				continue;
			case 'p':       /* --password */
				challenge_password = chunk_create(arg, strlen(arg));
				continue;
			case 'f':       /* --outform */
				if (!get_form(arg, &form, CRED_CERTIFICATE))
				{
					error = "invalid output format";
					goto usage;
				}
				continue;
			case 'x':       /* --keyid */
				keyid = arg;
				continue;
			case 'o':       /* --oldreq */
				oldreq_file = arg;
				continue;
			case EOF:
				break;
			default:
				error = "invalid --req option";
				goto usage;
		}
		break;
	}

	if (!dn && !oldreq_file)
	{
		error = "--dn or --oldreq is required";
		goto usage;
	}

	if (file)
	{
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
									 BUILD_FROM_FILE, file, BUILD_END);
	}
	else if (keyid)
	{
		chunk_t chunk;

		chunk = chunk_from_hex(chunk_create(keyid, strlen(keyid)), NULL);
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_ANY,
									  BUILD_PKCS11_KEYID, chunk, BUILD_END);
		free(chunk.ptr);
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
	scheme = get_signature_scheme(private, digest, pss);
	if (!scheme)
	{
		error = "no signature scheme found";
		goto end;
	}

	if (oldreq_file)
	{
		oldreq = lib->creds->create(lib->creds, CRED_CERTIFICATE,
									CERT_PKCS10_REQUEST,
									BUILD_FROM_FILE, oldreq_file, BUILD_END);
		if (!oldreq)
		{
			error = "parsing certificate request failed";
			goto end;
		}
		pkcs10 = (pkcs10_t*)oldreq;
		cert = pkcs10->replace_key(pkcs10, private, scheme, challenge_password);
		if (!cert)
		{
			error = "key replacement in certificate request failed";
			oldreq->destroy(oldreq);
			goto end;
		}
	}
	else
	{
		id = identification_create_from_string(dn);
		if (id->get_type(id) != ID_DER_ASN1_DN)
		{
			error = "supplied --dn is not a distinguished name";
			goto end;
		}
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_PKCS10_REQUEST,
								  BUILD_SIGNING_KEY, private,
								  BUILD_SUBJECT, id,
								  BUILD_SUBJECT_ALTNAMES, san,
								  BUILD_CHALLENGE_PWD, challenge_password,
								  BUILD_CERT_TYPE_EXT, cert_type_ext,
								  BUILD_SIGNATURE_SCHEME, scheme,
								  BUILD_END);
		if (!cert)
		{
			error = "generating certificate request failed";
			goto end;
		}
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
	signature_params_destroy(scheme);
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
		{"[--in file|--keyid hex] [--type rsa|ecdsa|bliss|priv]",
		 " --oldreq file|--dn distinguished-name [--san subjectAltName]+",
		 "[--profile server|client|dual|ocsp] [--password challengePassword]",
		 "[--digest sha1|sha224|sha256|sha384|sha512|sha3_224|sha3_256|sha3_384|sha3_512]",
		 "[--rsa-padding pkcs1|pss] [--outform der|pem]"},
		{
			{"help",        'h', 0, "show usage information"},
			{"in",          'i', 1, "private key input file, default: stdin"},
			{"keyid",       'x', 1, "smartcard or TPM private key object handle"},
			{"type",        't', 1, "type of input key, default: priv"},
			{"oldreq",      'o', 1, "old certificate request to be used as a template"},
			{"dn",          'd', 1, "subject distinguished name"},
			{"san",         'a', 1, "subjectAltName to include in cert request"},
			{"profile",     'P', 1, "certificate profile name to include in cert request"},
			{"password",    'p', 1, "challengePassword to include in cert request"},
			{"digest",      'g', 1, "digest for signature creation, default: key-specific"},
			{"rsa-padding", 'R', 1, "padding for RSA signatures, default: pkcs1"},
			{"outform",     'f', 1, "encoding of generated request, default: der"},
		}
	});
}
