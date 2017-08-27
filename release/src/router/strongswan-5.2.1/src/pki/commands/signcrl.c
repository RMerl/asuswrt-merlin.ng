/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "pki.h"

#include <utils/debug.h>
#include <collections/linked_list.h>
#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>
#include <credentials/certificates/crl.h>


/**
 * Entry for a revoked certificate
 */
typedef struct {
	chunk_t serial;
	crl_reason_t reason;
	time_t date;
} revoked_t;

/**
 * Add a revocation to the list
 */
static void add_revoked(linked_list_t *list,
						chunk_t serial, crl_reason_t reason, time_t date)
{
	revoked_t *revoked;

	INIT(revoked,
		.serial = chunk_clone(serial),
		.reason = reason,
		.date = date,
	);
	list->insert_last(list, revoked);
}

/**
 * Destroy a reason entry
 */
static void revoked_destroy(revoked_t *revoked)
{
	free(revoked->serial.ptr);
	free(revoked);
}

/**
 * Filter for revoked enumerator
 */
static bool filter(void *data, revoked_t **revoked, chunk_t *serial, void *p2,
				   time_t *date, void *p3, crl_reason_t *reason)
{
	*serial = (*revoked)->serial;
	*date = (*revoked)->date;
	*reason = (*revoked)->reason;
	return TRUE;
}

/**
 * Extract the serial of a certificate, write it into buf
 */
static int read_serial(char *file, char *buf, int buflen)
{
	certificate_t *cert;
	x509_t *x509;
	chunk_t serial;

	x509 = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_FROM_FILE, file, BUILD_END);
	cert = &x509->interface;
	if (!cert)
	{
		return -1;
	}
	serial = x509->get_serial(x509);
	if (serial.len == 0 || serial.len > buflen)
	{
		cert->destroy(cert);
		return -2;
	}
	memcpy(buf, serial.ptr, serial.len);
	cert->destroy(cert);
	return serial.len;
}

/**
 * Destroy a CDP
 */
static void cdp_destroy(x509_cdp_t *this)
{
	free(this->uri);
	free(this);
}

/**
 * Sign a CRL
 */
static int sign_crl()
{
	cred_encoding_type_t form = CERT_ASN1_DER;
	private_key_t *private = NULL;
	public_key_t *public = NULL;
	certificate_t *ca = NULL, *crl = NULL;
	crl_t *lastcrl = NULL;
	x509_t *x509;
	hash_algorithm_t digest = HASH_SHA1;
	char *arg, *cacert = NULL, *cakey = NULL, *lastupdate = NULL, *error = NULL;
	char *basecrl = NULL;
	char serial[512], *keyid = NULL;
	int serial_len = 0;
	crl_reason_t reason = CRL_REASON_UNSPECIFIED;
	time_t thisUpdate, nextUpdate, date = time(NULL);
	time_t lifetime = 15 * 24 * 60 * 60;
	char *datetu = NULL, *datenu = NULL, *dateform = NULL;
	linked_list_t *list, *cdps;
	enumerator_t *enumerator, *lastenum = NULL;
	x509_cdp_t *cdp;
	chunk_t crl_serial = chunk_empty, baseCrlNumber = chunk_empty;
	chunk_t encoding = chunk_empty;

	list = linked_list_create();
	cdps = linked_list_create();

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
			case 'c':
				cacert = arg;
				continue;
			case 'k':
				cakey = arg;
				continue;
			case 'x':
				keyid = arg;
				continue;
			case 'a':
				lastupdate = arg;
				continue;
			case 'l':
				lifetime = atoi(arg) * 24 * 60 * 60;
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
				datetu = arg;
				continue;
			case 'T':
				datenu = arg;
				continue;
			case 'z':
				serial_len = read_serial(arg, serial, sizeof(serial));
				if (serial_len < 0)
				{
					snprintf(serial, sizeof(serial),
							 "parsing certificate '%s' failed", arg);
					error = serial;
					goto error;
				}
				add_revoked(list, chunk_create(serial, serial_len), reason, date);
				date = time(NULL);
				serial_len = 0;
				reason = CRL_REASON_UNSPECIFIED;
				continue;
			case 's':
			{
				chunk_t chunk;
				int hex_len;

				hex_len = strlen(arg);
				if ((hex_len / 2) + (hex_len % 2) > sizeof(serial))
				{
					error = "invalid serial";
					goto usage;
				}
				chunk = chunk_from_hex(chunk_create(arg, hex_len), serial);
				serial_len = chunk.len;
				add_revoked(list, chunk_create(serial, serial_len), reason, date);
				date = time(NULL);
				serial_len = 0;
				reason = CRL_REASON_UNSPECIFIED;
				continue;
			}
			case 'b':
				basecrl = arg;
				continue;
			case 'u':
				INIT(cdp,
					.uri = strdup(arg),
				);
				cdps->insert_last(cdps, cdp);
				continue;
			case 'r':
				if (streq(arg, "key-compromise"))
				{
					reason = CRL_REASON_KEY_COMPROMISE;
				}
				else if (streq(arg, "ca-compromise"))
				{
					reason = CRL_REASON_CA_COMPROMISE;
				}
				else if (streq(arg, "affiliation-changed"))
				{
					reason = CRL_REASON_AFFILIATION_CHANGED;
				}
				else if (streq(arg, "superseded"))
				{
					reason = CRL_REASON_SUPERSEDED;
				}
				else if (streq(arg, "cessation-of-operation"))
				{
					reason = CRL_REASON_CESSATION_OF_OPERATON;
				}
				else if (streq(arg, "certificate-hold"))
				{
					reason = CRL_REASON_CERTIFICATE_HOLD;
				}
				else
				{
					error = "invalid revocation reason";
					goto usage;
				}
				continue;
			case 'd':
				date = atol(arg);
				if (!date)
				{
					error = "invalid date";
					goto usage;
				}
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
				error = "invalid --signcrl option";
				goto usage;
		}
		break;
	}

	if (!cacert)
	{
		error = "--cacert is required";
		goto usage;
	}
	if (!cakey && !keyid)
	{
		error = "--cakey or --keyid is required";
		goto usage;
	}
	if (!calculate_lifetime(dateform, datetu, datenu, lifetime,
							&thisUpdate, &nextUpdate))
	{
		error = "invalid --this/next-update datetime";
		goto usage;
	}

	ca = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							BUILD_FROM_FILE, cacert, BUILD_END);
	if (!ca)
	{
		error = "parsing CA certificate failed";
		goto error;
	}
	x509 = (x509_t*)ca;
	if (!(x509->get_flags(x509) & (X509_CA | X509_CRL_SIGN)))
	{
		error = "CA certificate misses CA basicConstraint / CRLSign keyUsage";
		goto error;
	}
	public = ca->get_public_key(ca);
	if (!public)
	{
		error = "extracting CA certificate public key failed";
		goto error;
	}
	if (cakey)
	{
		private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY,
									 public->get_type(public),
									 BUILD_FROM_FILE, cakey, BUILD_END);
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
		error = "loading CA private key failed";
		goto error;
	}
	if (!private->belongs_to(private, public))
	{
		error = "CA private key does not match CA certificate";
		goto error;
	}

	if (basecrl)
	{
		lastcrl = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_CRL,
									 BUILD_FROM_FILE, basecrl, BUILD_END);
		if (!lastcrl)
		{
			error = "loading base CRL failed";
			goto error;
		}
		baseCrlNumber = chunk_clone(lastcrl->get_serial(lastcrl));
		crl_serial = baseCrlNumber;
		DESTROY_IF((certificate_t*)lastcrl);
		lastcrl = NULL;
	}

	if (lastupdate)
	{
		lastcrl = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_CRL,
									 BUILD_FROM_FILE, lastupdate, BUILD_END);
		if (!lastcrl)
		{
			error = "loading lastUpdate CRL failed";
			goto error;
		}
		crl_serial = lastcrl->get_serial(lastcrl);
		lastenum = lastcrl->create_enumerator(lastcrl);
	}
	else
	{
		crl_serial = chunk_from_chars(0x00);
		lastenum = enumerator_create_empty();
	}

	/* remove superfluous leading zeros */
	while (crl_serial.len > 1 && crl_serial.ptr[0] == 0x00 &&
		  (crl_serial.ptr[1] & 0x80) == 0x00)
	{
		crl_serial = chunk_skip_zero(crl_serial);
	}
	crl_serial = chunk_clone(crl_serial);

	/* increment the serial number by one */
	chunk_increment(crl_serial);

	enumerator = enumerator_create_filter(list->create_enumerator(list),
										  (void*)filter, NULL, NULL);
	crl = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_CRL,
			BUILD_SIGNING_KEY, private, BUILD_SIGNING_CERT, ca,
			BUILD_SERIAL, crl_serial,
			BUILD_NOT_BEFORE_TIME, thisUpdate, BUILD_NOT_AFTER_TIME, nextUpdate,
			BUILD_REVOKED_ENUMERATOR, enumerator,
			BUILD_REVOKED_ENUMERATOR, lastenum, BUILD_DIGEST_ALG, digest,
			BUILD_CRL_DISTRIBUTION_POINTS, cdps, BUILD_BASE_CRL, baseCrlNumber,
			BUILD_END);
	enumerator->destroy(enumerator);
	lastenum->destroy(lastenum);
	DESTROY_IF((certificate_t*)lastcrl);
	free(crl_serial.ptr);

	if (!crl)
	{
		error = "generating CRL failed";
		goto error;
	}
	if (!crl->get_encoding(crl, form, &encoding))
	{
		error = "encoding CRL failed";
		goto error;
	}
	set_file_mode(stdout, form);
	if (fwrite(encoding.ptr, encoding.len, 1, stdout) != 1)
	{
		error = "writing CRL failed";
		goto error;
	}

error:
	DESTROY_IF(public);
	DESTROY_IF(private);
	DESTROY_IF(ca);
	DESTROY_IF(crl);
	free(encoding.ptr);
	free(baseCrlNumber.ptr);
	list->destroy_function(list, (void*)revoked_destroy);
	cdps->destroy_function(cdps, (void*)cdp_destroy);
	if (error)
	{
		fprintf(stderr, "%s\n", error);
		return 1;
	}
	return 0;

usage:
	list->destroy_function(list, (void*)revoked_destroy);
	cdps->destroy_function(cdps, (void*)cdp_destroy);
	return command_usage(error);
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		sign_crl, 'c', "signcrl",
		"issue a CRL using a CA certificate and key",
		{"--cacert file --cakey file|--cakeyid hex [--lifetime days]",
		 "  [--lastcrl crl] [--basecrl crl] [--crluri uri]+",
		 "  [[--reason key-compromise|ca-compromise|affiliation-changed|",
		 "             superseded|cessation-of-operation|certificate-hold]",
		 "   [--date timestamp] --cert file|--serial hex]*",
		 "  [--digest md5|sha1|sha224|sha256|sha384|sha512]",
		 "  [--outform der|pem]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"cacert",		'c', 1, "CA certificate file"},
			{"cakey",		'k', 1, "CA private key file"},
			{"cakeyid",		'x', 1, "keyid on smartcard of CA private key"},
			{"lifetime",	'l', 1, "days the CRL gets a nextUpdate, default: 15"},
			{"this-update",	'F', 1, "date/time the validity of the CRL starts"},
			{"next-update",	'T', 1, "date/time the validity of the CRL ends"},
			{"dateform",	'D', 1, "strptime(3) input format, default: %d.%m.%y %T"},
			{"lastcrl",		'a', 1, "CRL of lastUpdate to copy revocations from"},
			{"basecrl",		'b', 1, "base CRL to create a delta CRL for"},
			{"crluri",		'u', 1, "freshest delta CRL URI to include"},
			{"cert",		'z', 1, "certificate file to revoke"},
			{"serial",		's', 1, "hex encoded certificate serial number to revoke"},
			{"reason",		'r', 1, "reason for certificate revocation"},
			{"date",		'd', 1, "revocation date as unix timestamp, default: now"},
			{"digest",		'g', 1, "digest for signature creation, default: sha1"},
			{"outform",		'f', 1, "encoding of generated crl, default: der"},
		}
	});
}
