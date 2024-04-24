/*
 * Copyright (C) 2023 Tobias Brunner
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

#include "index_responder.h"

#include <asn1/asn1.h>
#include <collections/hashtable.h>

typedef struct private_ocsp_responder_t private_ocsp_responder_t;

/**
 * Private data
 */
struct private_ocsp_responder_t {

	/**
	 * Public interface
	 */
	ocsp_responder_t public;

	/**
	 * CA certificate
	 */
	certificate_t *ca;

	/**
	 * Certificate database
	 */
	hashtable_t *certs;
};

/**
 * Status information on a certificate
 */
typedef struct {
	/** Serial number */
	chunk_t serial;
	/** Certificate's validity */
	cert_validation_t validation;
	/** Revocation reason */
	crl_reason_t reason;
	/** Revocation time */
	time_t revocation_time;
} cert_entry_t;

/**
 * Destroy a certificate status entry
 */
static void destroy_cert_entry(cert_entry_t *this)
{
	chunk_free(&this->serial);
	free(this);
}

METHOD(ocsp_responder_t, get_status, cert_validation_t,
	private_ocsp_responder_t *this, certificate_t *cacert,
	chunk_t serial_number, time_t *revocation_time, crl_reason_t *reason)
{
	cert_entry_t *cert;

	if (!cacert->equals(cacert, this->ca))
	{
		return VALIDATION_SKIPPED;
	}
	cert = this->certs->get(this->certs, &serial_number);
	if (!cert)
	{
		return VALIDATION_FAILED;
	}
	if (revocation_time)
	{
		*revocation_time = cert->revocation_time;
	}
	if (reason)
	{
		*reason = cert->reason;
	}
	return cert->validation;
}

METHOD(ocsp_responder_t, destroy, void,
	private_ocsp_responder_t *this)
{
	lib->ocsp->remove_responder(lib->ocsp, &this->public);
	this->certs->destroy_function(this->certs, (void*)destroy_cert_entry);
	this->ca->destroy(this->ca);
	free(this);
}

/*
 * Described in header
 */
ocsp_responder_t *index_responder_create(certificate_t *ca, char *path)
{
	private_ocsp_responder_t *this;
	hashtable_t *certs;
	char line[BUF_LEN], *token, *pos, *reason_str;
	FILE *file;

	file = fopen(path, "r");
	if (!file)
	{
		fprintf(stderr, "failed to open '%s': %s\n", path, strerror(errno));
		return NULL;
	}
	certs = hashtable_create((hashtable_hash_t)chunk_hash_ptr,
							 (hashtable_equals_t)chunk_equals_ptr, 32);
	while (fgets(line, sizeof(line), file))
	{
		enumerator_t *enumerator;
		cert_entry_t *cert;
		cert_validation_t validation;
		crl_reason_t reason = CRL_REASON_UNSPECIFIED;
		time_t revocation_time = 0;
		chunk_t revocation, serial;
		bool valid = FALSE;
		int i = 0;

		switch (line[0])
		{
			case 'E':
				/* OpenSSL only converts valid entries to expired ones */
			case 'V':
				validation = VALIDATION_GOOD;
				break;
			case 'R':
				validation = VALIDATION_REVOKED;
				break;
			default:
				/* ignore comments, empty lines etc. */
				continue;
		}
		enumerator = enumerator_create_token(&line[1], "\t", " \n\r");
		while (enumerator->enumerate(enumerator, &token))
		{
			switch (i++)
			{
				case 0: /* expiration date/time in UTC (YYMMDDHHMMSSZ), ignored */
					continue;
				case 1: /* if revoked, revocation date/time and optional reason */
					if (validation == VALIDATION_REVOKED)
					{
						reason_str = NULL;
						pos = strchr(token, ',');
						if (pos)
						{
							*pos = '\0';
							reason_str = ++pos;
							/* OpenSSL may optionally store an OID if the reason
							 * is certificateHold (hold instruction code in
							 * RFC 3280, was removed with RFC 5280) */
							pos = strchr(pos, ',');
							if (pos)
							{
								*pos = '\0';
							}
						}
						revocation = chunk_from_str(token);
						revocation_time = asn1_to_time(&revocation,
													   ASN1_UTCTIME);
						if (!revocation_time)
						{
							break;
						}
						if (strcaseeq(reason_str, "keyCompromise"))
						{
							reason = CRL_REASON_KEY_COMPROMISE;
						}
						else if (strcaseeq(reason_str, "CACompromise"))
						{
							reason = CRL_REASON_CA_COMPROMISE;
						}
						else if (strcaseeq(reason_str, "affiliationChanged"))
						{
							reason = CRL_REASON_AFFILIATION_CHANGED;
						}
						else if (strcaseeq(reason_str, "superseded"))
						{
							reason = CRL_REASON_SUPERSEDED;
						}
						else if (strcaseeq(reason_str, "cessationOfOperation"))
						{
							reason = CRL_REASON_CESSATION_OF_OPERATION;
						}
						else if (strcaseeq(reason_str, "certificateHold"))
						{
							reason = CRL_REASON_CERTIFICATE_HOLD;
							validation = VALIDATION_ON_HOLD;
						}
						else if (strcaseeq(reason_str, "removeFromCRL"))
						{
							reason = CRL_REASON_REMOVE_FROM_CRL;
						}
						else
						{
							reason = CRL_REASON_UNSPECIFIED;
						}
						continue;
					}
					i++;
					/* if not revoked, this field is empty, fall-through */
				case 2: /* hexadecimal serial number */
					serial = chunk_from_hex(chunk_from_str(token), NULL);
					valid = serial.len > 0;
					/* skip the last two fields, an optional path, usually set
					 * to "unknown", and the subject DN (RDNs separated by
					 * slashes), which we don't use */
					break;
				default:
					break;
			}
			break;
		}
		enumerator->destroy(enumerator);

		if (valid)
		{
			INIT(cert,
				.serial = serial,
				.validation = validation,
				.reason = reason,
				.revocation_time = revocation_time,
			);
			cert = certs->put(certs, &cert->serial, cert);
			if (cert)
			{
				destroy_cert_entry(cert);
			}
		}
	}
	fclose(file);

	INIT(this,
		.public = {
			.get_status = _get_status,
			.destroy = _destroy,
		},
		.ca = ca->get_ref(ca),
		.certs = certs,
	);
	lib->ocsp->add_responder(lib->ocsp, &this->public);

	DBG1(DBG_APP, "loaded status of %u certificates issued by '%Y' from %s",
		 certs->get_count(certs), ca->get_subject(ca), path);
	return &this->public;
}
