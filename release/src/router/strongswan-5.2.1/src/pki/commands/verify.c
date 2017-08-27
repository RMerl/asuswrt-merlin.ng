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
#include <credentials/sets/mem_cred.h>

/**
 * Verify a certificate signature
 */
static int verify()
{
	bool trusted = FALSE, valid = FALSE, revoked = FALSE;
	bool has_ca = FALSE, online = FALSE;
	certificate_t *cert;
	enumerator_t *enumerator;
	auth_cfg_t *auth;
	mem_cred_t *creds;
	char *arg, *file = NULL;

	creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &creds->set);

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				creds->destroy(creds);
				return command_usage(NULL);
			case 'i':
				file = arg;
				continue;
			case 'c':
				cert = lib->creds->create(lib->creds,
										  CRED_CERTIFICATE, CERT_X509,
										  BUILD_FROM_FILE, arg, BUILD_END);
				if (!cert)
				{
					fprintf(stderr, "parsing CA certificate failed\n");
					goto end;
				}
				has_ca = TRUE;
				creds->add_cert(creds, TRUE, cert);
				continue;
			case 'o':
				online = TRUE;
				continue;
			case EOF:
				break;
			default:
				creds->destroy(creds);
				return command_usage("invalid --verify option");
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
			fprintf(stderr, "reading certificate failed: %s\n", strerror(errno));
			goto end;
		}
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_BLOB, chunk, BUILD_END);
		free(chunk.ptr);
	}
	if (!cert)
	{
		fprintf(stderr, "parsing certificate failed\n");
		goto end;
	}
	creds->add_cert(creds, !has_ca, cert);

	enumerator = lib->credmgr->create_trusted_enumerator(lib->credmgr,
									KEY_ANY, cert->get_subject(cert), online);
	if (enumerator->enumerate(enumerator, &cert, &auth))
	{
		trusted = TRUE;
		if (cert->get_validity(cert, NULL, NULL, NULL))
		{
			printf("certificate trusted, lifetimes valid");
			valid = TRUE;
		}
		else
		{
			printf("certificate trusted, but no valid lifetime");
		}
		if (online)
		{
			switch ((uintptr_t)auth->get(auth, AUTH_RULE_CRL_VALIDATION))
			{
				case VALIDATION_GOOD:
					printf(", certificate not revoked");
					break;
				case VALIDATION_SKIPPED:
					printf(", no revocation information");
					break;
				case VALIDATION_STALE:
					printf(", revocation information stale");
					break;
				case VALIDATION_FAILED:
					printf(", revocation checking failed");
					break;
				case VALIDATION_ON_HOLD:
					printf(", certificate revocation on hold");
					revoked = TRUE;
					break;
				case VALIDATION_REVOKED:
					printf(", certificate revoked");
					revoked = TRUE;
					break;
			}
		}
		printf("\n");
	}
	enumerator->destroy(enumerator);

	if (!trusted)
	{
		printf("certificate untrusted\n");
	}

end:
	lib->credmgr->remove_set(lib->credmgr, &creds->set);
	creds->destroy(creds);

	if (!trusted)
	{
		return 1;
	}
	if (!valid)
	{
		return 2;
	}
	if (revoked)
	{
		return 3;
	}
	return 0;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		verify, 'v', "verify",
		"verify a certificate using the CA certificate",
		{"[--in file] [--cacert file]"},
		{
			{"help",	'h', 0, "show usage information"},
			{"in",		'i', 1, "X.509 certificate to verify, default: stdin"},
			{"cacert",	'c', 1, "CA certificate for trustchain verification"},
			{"online",	'o', 0, "enable online CRL/OCSP revocation checking"},
		}
	});
}
