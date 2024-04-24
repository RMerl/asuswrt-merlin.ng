/*
 * Copyright (C) 2022 Andreas Steffen, strongSec GmbH
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

#include "pki.h"
#include "pki_cert.h"
#include "est/est_tls.h"

#include <credentials/containers/pkcs7.h>
#include <credentials/certificates/certificate.h>
#include <credentials/sets/mem_cred.h>

/**
 * Get CA certificate[s] from an EST server (RFC 7030)
 */
static int estca()
{
	cred_encoding_type_t form = CERT_ASN1_DER;
	chunk_t est_response = chunk_empty;
	certificate_t *cacert;
	mem_cred_t *creds = NULL;
	est_tls_t *est_tls;
	char *arg, *error = NULL, *url = NULL, *label = NULL, *caout = NULL;
	bool force = FALSE, success;
	u_int http_code = 0;
	status_t status = 1;

	/* initialize CA certificate storage */
	creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &creds->set);

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':       /* --help */
				goto usage;
			case 'u':       /* --url */
				url = arg;
				continue;
			case 'l':       /* --label */
				label = arg;
				continue;
			case 'C':       /* --cacert */
				cacert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
							 CERT_X509,	BUILD_FROM_FILE, arg, BUILD_END);
				if (!cacert)
				{
					DBG1(DBG_APP, "could not load cacert file '%s'", arg);
					goto err;
				}
				creds->add_cert(creds, TRUE, cacert);
				continue;
			case 'c':       /* --caout */
				caout = arg;
				continue;
			case 'f':       /* --outform */
				if (!get_form(arg, &form, CRED_CERTIFICATE))
				{
					error ="invalid certificate output format";
					goto usage;
				}
				continue;
			case 'F':       /* --force */
				force = TRUE;
				continue;
			case EOF:
				break;
			default:
				error ="invalid --estca option";
				goto usage;
		}
		break;
	}

	if (!url)
	{
		return command_usage("--url is required");
	}

	est_tls = est_tls_create(url, label, NULL, NULL);
	if (!est_tls)
	{
		DBG1(DBG_APP, "TLS connection to EST server was not established");
		goto err;
	}
	success = est_tls->request(est_tls, EST_CACERTS, chunk_empty, &est_response,
							   &http_code, NULL);
	est_tls->destroy(est_tls);

	if (!success)
	{
		DBG1(DBG_APP, "EST request failed: HTTP %u", http_code);
		goto err;
	}
	if (pki_cert_extract_cacerts(est_response, caout, NULL, TRUE, form, force))
	{
		status = 0;
	}

err:
	lib->credmgr->remove_set(lib->credmgr, &creds->set);
	creds->destroy(creds);
	chunk_free(&est_response);

	return status;

usage:
	lib->credmgr->remove_set(lib->credmgr, &creds->set);
	creds->destroy(creds);

	return command_usage(error);
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		estca, 'e', "estca",
		"get CA certificate[s] from an EST server",
		{"--url url [--label label] --cacert file [--caout file]",
		 "[--outform der|pem] [--force]"},
		{
			{"help",    'h', 0, "show usage information"},
			{"url",     'u', 1, "URL of the EST server"},
			{"label",   'l', 1, "label in the EST server path"},
			{"cacert",  'C', 1, "TLS CA certificate(s)"},
			{"caout",   'c', 1, "CA certificate [template]"},
			{"outform", 'f', 1, "encoding of stored certificates, default: der"},
			{"force",   'F', 0, "force overwrite of existing files"},
		}
	});
}
