/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * Copyright (C) 2015 Andreas Steffen
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

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include <asn1/asn1.h>
#include <asn1/oid.h>
#include <credentials/certificates/certificate.h>
#include <credentials/certificates/certificate_printer.h>
#include <selectors/traffic_selector.h>

#include "command.h"

/**
 * Static certificate printer object
 */
static certificate_printer_t *cert_printer = NULL;

/**
 * Print PEM encoding of a certificate
 */
static void print_pem(certificate_t *cert)
{
	chunk_t encoding;

	if (cert->get_encoding(cert, CERT_PEM, &encoding))
	{
		printf("%.*s", (int)encoding.len, encoding.ptr);
		free(encoding.ptr);
	}
	else
	{
		fprintf(stderr, "PEM encoding certificate failed\n");
	}
}

CALLBACK(list_cb, void,
	command_format_options_t *format, char *name, vici_res_t *res)
{
	certificate_t *cert;
	certificate_type_t type;
	x509_flag_t flag = X509_NONE;
	identification_t *subject = NULL;
	time_t not_before = UNDEFINED_TIME;
	time_t not_after  = UNDEFINED_TIME;
	chunk_t t_ch;
	bool has_privkey;
	char *str;
	void *buf;
	int len;

	if (*format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "list-cert event", *format & COMMAND_FORMAT_PRETTY,
				  stdout);
		return;
	}

	buf = vici_find(res, &len, "data");
	if (!buf)
	{
		fprintf(stderr, "received incomplete certificate data\n");
		return;
	}
	has_privkey = streq(vici_find_str(res, "no", "has_privkey"), "yes");

	str = vici_find_str(res, "ANY", "type");
	if (!enum_from_name(certificate_type_names, str, &type) || type == CERT_ANY)
	{
		fprintf(stderr, "unsupported certificate type '%s'\n", str);
		return;
	}
	if (type == CERT_X509)
	{
		str = vici_find_str(res, "ANY", "flag");
		if (!enum_from_name(x509_flag_names, str, &flag) || flag == X509_ANY)
		{
			fprintf(stderr, "unsupported certificate flag '%s'\n", str);
			return;
		}
	}
	if (type == CERT_TRUSTED_PUBKEY)
	{
		str = vici_find_str(res, NULL, "subject");
		if (str)
		{
			subject = identification_create_from_string(str);
		}
		str = vici_find_str(res, NULL, "not-before");
		if (str)
		{
			t_ch = chunk_from_str(str);
			not_before = asn1_to_time(&t_ch, ASN1_GENERALIZEDTIME);
		}
		str = vici_find_str(res, NULL, "not-after");
		if (str)
		{
			t_ch = chunk_from_str(str);
			not_after = asn1_to_time(&t_ch, ASN1_GENERALIZEDTIME);
		}
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, type,
								  BUILD_BLOB_ASN1_DER, chunk_create(buf, len),
								  BUILD_NOT_BEFORE_TIME, not_before,
								  BUILD_NOT_AFTER_TIME, not_after,
								  BUILD_SUBJECT, subject, BUILD_END);
		DESTROY_IF(subject);
	}
	else
	{
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, type,
								  BUILD_BLOB_ASN1_DER, chunk_create(buf, len),
								  BUILD_END);
	}
	if (cert)
	{
		if (*format & COMMAND_FORMAT_PEM)
		{
			print_pem(cert);
		}
		else
		{
			cert_printer->print_caption(cert_printer, type, flag);
			cert_printer->print(cert_printer, cert, has_privkey);
		}
		cert->destroy(cert);
	}
	else
	{
		fprintf(stderr, "parsing certificate failed\n");
	}
}

static int list_certs(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *subject = NULL, *type = NULL, *flag = NULL;
	bool detailed = TRUE, utc = FALSE;
	int ret;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 's':
				subject = arg;
				continue;
			case 't':
				type = arg;
				continue;
			case 'f':
				flag = arg;
				continue;
			case 'p':
				format |= COMMAND_FORMAT_PEM;
				continue;
			case 'P':
				format |= COMMAND_FORMAT_PRETTY;
				/* fall through to raw */
			case 'r':
				format |= COMMAND_FORMAT_RAW;
				continue;
			case 'S':
				detailed = FALSE;
				continue;
			case 'U':
				utc = TRUE;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --list-certs option");
		}
		break;
	}
	if (vici_register(conn, "list-cert", list_cb, &format) != 0)
	{
		ret = errno;
		fprintf(stderr, "registering for certificates failed: %s\n",
				strerror(errno));
		return ret;
	}
	req = vici_begin("list-certs");

	if (type)
	{
		vici_add_key_valuef(req, "type", "%s", type);
	}
	if (flag)
	{
		vici_add_key_valuef(req, "flag", "%s", flag);
	}
	if (subject)
	{
		vici_add_key_valuef(req, "subject", "%s", subject);
	}
	cert_printer = certificate_printer_create(stdout, detailed, utc);

	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "list-certs request failed: %s\n", strerror(errno));
		cert_printer->destroy(cert_printer);
		cert_printer = NULL;
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "list-certs reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	vici_free_res(res);

	cert_printer->destroy(cert_printer);
	cert_printer = NULL;
	return 0;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		list_certs, 'x', "list-certs", "list stored certificates",
		{"[--subject <dn/san>] [--pem]",
		 "[--type x509|x509_ac|x509_crl|ocsp_response|pubkey]",
		 "[--flag none|ca|aa|ocsp|any] [--raw|--pretty|--short|--utc]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"subject",		's', 1, "filter by certificate subject"},
			{"type",		't', 1, "filter by certificate type"},
			{"flag",		'f', 1, "filter by X.509 certificate flag"},
			{"pem",			'p', 0, "print PEM encoding of certificate"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
			{"short",		'S', 0, "omit some certificate details"},
			{"utc",			'U', 0, "use UTC for time fields"},
		}
	});
}
