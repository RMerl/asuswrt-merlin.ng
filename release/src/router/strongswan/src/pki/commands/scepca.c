/*
 * Copyright (C) 2005 Jan Hutter, Martin Willi
 * Copyright (C) 2012 Tobias Brunner
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
#include "scep/scep.h"

#include <credentials/certificates/certificate.h>

/**
 * Get CA certificate[s] from a SCEP server (RFC 8894)
 */
static int scepca()
{
	cred_encoding_type_t form = CERT_ASN1_DER;
	chunk_t scep_response = chunk_empty;
	char *arg, *url = NULL, *caout = NULL, *raout = NULL;
	bool force = FALSE, success;
	u_int http_code = 0;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':       /* --help */
				return command_usage(NULL);
			case 'u':       /* --url */
				url = arg;
				continue;
			case 'c':       /* --caout */
				caout = arg;
				continue;
			case 'r':       /* --raout */
				raout = arg;
				continue;
			case 'f':       /* --form */
				if (!get_form(arg, &form, CRED_CERTIFICATE))
				{
					return command_usage("invalid certificate output format");
				}
				continue;
			case 'F':       /* --force */
				force = TRUE;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --scepca option");
		}
		break;
	}

	if (!url)
	{
		return command_usage("--url is required");
	}

	if (!scep_http_request(url, SCEP_GET_CA_CERT, FALSE, chunk_empty,
						   &scep_response, &http_code))
	{
		DBG1(DBG_APP, "did not receive a valid SCEP response: HTTP %u", http_code);
		return 1;
	}

	success = pki_cert_extract_cacerts(scep_response, caout, raout, TRUE, form,
									   force);
	chunk_free(&scep_response);

	return success ? 0 : 1;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		scepca, 'C', "scepca",
		"get CA [and RA] certificate[s] from a SCEP server",
		{"--url url [--caout file] [--raout file] [--outform der|pem] [--force]"},
		{
			{"help",    'h', 0, "show usage information"},
			{"url",     'u', 1, "URL of the SCEP server"},
			{"caout",   'c', 1, "CA certificate [template]"},
			{"raout",   'r', 1, "RA certificate [template]"},
			{"outform", 'f', 1, "encoding of stored certificates, default: der"},
			{"force",   'F', 0, "force overwrite of existing files"},
		}
	});
}
