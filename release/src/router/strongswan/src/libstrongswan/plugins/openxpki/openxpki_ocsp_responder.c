/*
 * Copyright (C) 2023 Andreas Steffen, strongSec GmbH
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

#include <library.h>
#include <utils/debug.h>

#include "openxpki_ocsp_responder.h"

typedef struct private_openxpki_ocsp_responder_t private_openxpki_ocsp_responder_t;

/**
 * Private Data of a openxpki_ocsp_responder_t object.
 */
struct private_openxpki_ocsp_responder_t {

	/**
	 * Public data
	 */
	openxpki_ocsp_responder_t public;

	/**
	 * OpenXPKI certificate database
	 */
	database_t *db;
};

METHOD(ocsp_responder_t, get_status, cert_validation_t,
	private_openxpki_ocsp_responder_t *this, certificate_t *cacert,
	chunk_t serial_number, time_t *revocation_time, crl_reason_t *reason)
{
	cert_validation_t validation = VALIDATION_FAILED;
	int rev_time;
	const int max_decimals = 49;
	const int max_bytes = max_decimals / 2.4083;
	char serial[max_decimals + 1], authKeyId[60];
	char *status, *reason_code;
	chunk_t subjectKeyId;
	public_key_t *public;
	enumerator_t *e;
	bool success;

	/* convert serialNumber from binary to decimal as required for the DB query.
	 * check for a potential overflow since the database table field supports
	 * up to 49 decimal digits, only.
	 */
	if (serial_number.len > max_bytes)
	{
		DBG1(DBG_LIB, "serialNumber conversion exceeds %d decimals", max_decimals);
		return VALIDATION_FAILED;

	}
	chunk_to_dec(serial_number, serial);

	/* additionally use the subjectyKeyId of the issuing CA for the DB query */
	public = cacert->get_public_key(cacert);
	success = public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &subjectKeyId);
	public->destroy(public);

	if (!success)
	{
		DBG1(DBG_LIB, "failed to extract subjectKeyId from CA certificate");
		return VALIDATION_FAILED;
	}

	/* the authKeyId of the certificate is the subjectKeyId of the issuing CA */
	snprintf(authKeyId, sizeof(authKeyId), "%#B", &subjectKeyId);

	/* query the OpenXPKI certificate database */
	e = this->db->query(this->db, "SELECT status, reason_code, revocation_time "
		                "FROM certificate WHERE cert_key = ? "
						"AND authority_key_identifier = ?",	DB_TEXT, serial,
						DB_TEXT, authKeyId, DB_TEXT, DB_TEXT, DB_INT);
	if (e && e->enumerate(e, &status, &reason_code, &rev_time))
	{
		if (streq(status, "ISSUED") || streq(status, "ISSUANCE_PENDING"))
		{
			validation = VALIDATION_GOOD;
		}
		else if (streq(status, "UNKNOWN"))
		{
			validation = VALIDATION_FAILED;
		}
		else if (streq(status, "REVOKED") || streq(status, "HOLD") ||
				 streq(status, "CRL_ISSUANCE_PENDING"))
		{
			validation = VALIDATION_REVOKED;

			if (revocation_time)
			{
				*revocation_time = rev_time;
			}
			if (reason)
			{
				if (streq(reason_code, "unspecified"))
				{
					*reason = CRL_REASON_UNSPECIFIED;
				}
				else if (streq(reason_code, "keyCompromise"))
				{
					*reason = CRL_REASON_KEY_COMPROMISE;
				}
				else if (streq(reason_code, "CACompromise"))
				{
					*reason = CRL_REASON_CA_COMPROMISE;
				}
				else if (streq(reason_code, "affiliationChanged"))
				{
					*reason = CRL_REASON_AFFILIATION_CHANGED;
				}
				else if (streq(reason_code, "superseded"))
				{
					*reason = CRL_REASON_SUPERSEDED;
				}
				else if (streq(reason_code, "cessationOfOperation"))
				{
					*reason = CRL_REASON_CESSATION_OF_OPERATION;
				}
					else if (streq(reason_code, "certificateHold"))
				{
					*reason = CRL_REASON_CERTIFICATE_HOLD;
					validation = VALIDATION_ON_HOLD;
				}
				else if (streq(reason_code, "removeFromCRL"))
				{
					*reason = CRL_REASON_REMOVE_FROM_CRL;
				}
					else
				{
					*reason = CRL_REASON_UNSPECIFIED;
				}
			}
		}
	}
	DESTROY_IF(e);

	return validation;
}

METHOD(ocsp_responder_t, destroy, void,
	private_openxpki_ocsp_responder_t *this)
{
	this->db->destroy(this->db);
	free(this);
}

/*
 * See header
 */
ocsp_responder_t *openxpki_ocsp_responder_create()
{
	private_openxpki_ocsp_responder_t *this;
	database_t *db;
	char *uri;

	uri = lib->settings->get_str(lib->settings,
								 "%s.plugins.openxpki.database", NULL, lib->ns);
	if (!uri)
	{
		DBG1(DBG_CFG, "openxpki database URI missing");
		return NULL;
	}
	db = lib->db->create(lib->db, uri);
	if (!db)
	{
		DBG1(DBG_CFG, "opening openxpki database failed");
		return NULL;
	}
	INIT(this,
		.public = {
			.interface = {
				.get_status = _get_status,
				.destroy = _destroy,
			},
		},
		.db = db,
	);

	return &this->public.interface;
}
