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

#include <time.h>
#include <errno.h>

#include "pki.h"

#include <utils/debug.h>
#include <asn1/asn1.h>
#include <collections/linked_list.h>
#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>
#include <credentials/certificates/ac.h>

/**
 * Issue an attribute certificate
 */
static int acert()
{
	cred_encoding_type_t form = CERT_ASN1_DER;
	hash_algorithm_t digest = HASH_UNKNOWN;
	signature_params_t *scheme = NULL;
	certificate_t *ac = NULL, *cert = NULL, *issuer =NULL;
	private_key_t *private = NULL;
	public_key_t *public = NULL;
	char *file = NULL, *hex = NULL, *issuercert = NULL, *issuerkey = NULL;
	char *error = NULL, *keyid = NULL;
	linked_list_t *groups;
	chunk_t serial = chunk_empty, encoding = chunk_empty;
	time_t not_before, not_after, lifetime = 24 * 60 * 60;
	char *datenb = NULL, *datena = NULL, *dateform = NULL;
	rng_t *rng;
	char *arg;
	bool pss = lib->settings->get_bool(lib->settings, "%s.rsa_pss", FALSE,
									   lib->ns);

	groups = linked_list_create();

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				goto usage;
			case 'g':
				if (!enum_from_name(hash_algorithm_short_names, arg, &digest))
				{
					error = "invalid --digest type";
					goto usage;
				}
				continue;
			case 'R':
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
			case 'i':
				file = arg;
				continue;
			case 'm':
				groups->insert_last(groups, arg);
				continue;
			case 'c':
				issuercert = arg;
				continue;
			case 'k':
				issuerkey = arg;
				continue;
			case 'x':
				keyid = arg;
				continue;
			case 'l':
				lifetime = atoi(arg) * 60 * 60;
				if (!lifetime)
				{
					error = "invalid --lifetime value";
					goto usage;
				}
				continue;
			case 'D':
				dateform = arg;
				continue;
			case 'F':
				datenb = arg;
				continue;
			case 'T':
				datena = arg;
				continue;
			case 's':
				hex = arg;
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
				error = "invalid --acert option";
				goto usage;
		}
		break;
	}

	if (!calculate_lifetime(dateform, datenb, datena, lifetime,
							&not_before, &not_after))
	{
		error = "invalid --not-before/after datetime";
		goto usage;
	}

	if (!issuercert)
	{
		error = "--issuercert is required";
		goto usage;
	}
	if (!issuerkey && !keyid)
	{
		error = "--issuerkey or --issuerkeyid is required";
		goto usage;
	}

	issuer = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								BUILD_FROM_FILE, issuercert, BUILD_END);
	if (!issuer)
	{
		error = "parsing issuer certificate failed";
		goto end;
	}
	public = issuer->get_public_key(issuer);
	if (!public)
	{
		error = "extracting issuer certificate public key failed";
		goto end;
	}
	if (issuerkey)
	{
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY,
									 public->get_type(public),
									 BUILD_FROM_FILE, issuerkey, BUILD_END);
	}
	else
	{
		chunk_t chunk;

		chunk = chunk_from_hex(chunk_create(keyid, strlen(keyid)), NULL);
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_ANY,
									 BUILD_PKCS11_KEYID, chunk, BUILD_END);
		free(chunk.ptr);
	}
	if (!private)
	{
		error = "loading issuer private key failed";
		goto end;
	}
	if (!private->belongs_to(private, public))
	{
		error = "issuer private key does not match issuer certificate";
		goto end;
	}

	if (hex)
	{
		serial = chunk_from_hex(chunk_create(hex, strlen(hex)), NULL);
	}
	else
	{
		rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
		if (!rng)
		{
			error = "no random number generator found";
			goto end;
		}
		if (!rng_allocate_bytes_not_zero(rng, 8, &serial, FALSE))
		{
			error = "failed to generate serial number";
			rng->destroy(rng);
			goto end;
		}
		serial.ptr[0] &= 0x7F;
		rng->destroy(rng);
	}

	if (file)
	{
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_FROM_FILE, file, BUILD_END);
	}
	else
	{
		set_file_mode(stdin, CERT_ASN1_DER);
		if (!chunk_from_fd(0, &encoding))
		{
			fprintf(stderr, "%s: ", strerror(errno));
			error = "reading public key failed";
			goto end;
		}
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_BLOB, encoding, BUILD_END);
		chunk_free(&encoding);
	}
	if (!cert)
	{
		error = "parsing user certificate failed";
		goto end;
	}
	scheme = get_signature_scheme(private, digest, pss);
	if (!scheme)
	{
		error = "no signature scheme found";
		goto end;
	}

	ac = lib->creds->create(lib->creds,
							CRED_CERTIFICATE, CERT_X509_AC,
							BUILD_CERT, cert,
							BUILD_NOT_BEFORE_TIME, not_before,
							BUILD_NOT_AFTER_TIME, not_after,
							BUILD_SERIAL, serial,
							BUILD_AC_GROUP_STRINGS, groups,
							BUILD_SIGNING_CERT, issuer,
							BUILD_SIGNING_KEY, private,
							BUILD_SIGNATURE_SCHEME, scheme,
							BUILD_END);
	if (!ac)
	{
		error = "generating attribute certificate failed";
		goto end;
	}
	if (!ac->get_encoding(ac, form, &encoding))
	{
		error = "encoding attribute certificate failed";
		goto end;
	}
	set_file_mode(stdout, form);
	if (fwrite(encoding.ptr, encoding.len, 1, stdout) != 1)
	{
		error = "writing attribute certificate key failed";
		goto end;
	}

end:
	DESTROY_IF(ac);
	DESTROY_IF(cert);
	DESTROY_IF(issuer);
	DESTROY_IF(public);
	DESTROY_IF(private);
	groups->destroy(groups);
	signature_params_destroy(scheme);
	free(encoding.ptr);
	free(serial.ptr);

	if (error)
	{
		fprintf(stderr, "%s\n", error);
		return 1;
	}
	return 0;

usage:
	groups->destroy(groups);
	return command_usage(error);
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		acert, 'z', "acert",
		"issue an attribute certificate",
		{"[--in file] [--group name]* --issuerkey file|--issuerkeyid hex",
		 " --issuercert file [--serial hex] [--lifetime hours]",
		 " [--not-before datetime] [--not-after datetime] [--dateform form]",
		 "[--digest md5|sha1|sha224|sha256|sha384|sha512|sha3_224|sha3_256|sha3_384|sha3_512]",
		 "[--rsa-padding pkcs1|pss]",
		 "[--outform der|pem]"},
		{
			{"help",			'h', 0, "show usage information"},
			{"in",				'i', 1, "holder certificate, default: stdin"},
			{"group",			'm', 1, "group membership string to include"},
			{"issuercert",		'c', 1, "issuer certificate file"},
			{"issuerkey",		'k', 1, "issuer private key file"},
			{"issuerkeyid",		'x', 1, "smartcard or TPM issuer private key object handle"},
			{"serial",			's', 1, "serial number in hex, default: random"},
			{"lifetime",		'l', 1, "hours the acert is valid, default: 24"},
			{"not-before",		'F', 1, "date/time the validity of the AC starts"},
			{"not-after",		'T', 1, "date/time the validity of the AC ends"},
			{"dateform",		'D', 1, "strptime(3) input format, default: %d.%m.%y %T"},
			{"digest",			'g', 1, "digest for signature creation, default: key-specific"},
			{"rsa-padding",		'R', 1, "padding for RSA signatures, default: pkcs1"},
			{"outform",			'f', 1, "encoding of generated cert, default: der"},
		}
	});
}
