/*
 * Copyright (C) 2022-2023 Andreas Steffen, strongSec GmbH
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

#include "test_suite.h"

#include <credentials/certificates/x509.h>
#include <credentials/certificates/crl.h>
#include <credentials/certificates/ocsp_request.h>
#include <credentials/certificates/ocsp_response.h>
#include <credentials/certificates/ocsp_single_response.h>
#include <credentials/certificates/ac.h>

#include <time.h>

/**
 * RSA private key, so we don't have to generate one
 */
static char keydata[] = {
  0x30,0x82,0x02,0x5e,0x02,0x01,0x00,0x02,0x81,0x81,0x00,0xb1,0x9b,0xd4,0x51,0x24,
  0xfc,0x56,0x1d,0x3d,0xfb,0xa2,0xea,0x37,0x02,0x70,0x72,0x87,0x84,0x2f,0x3b,0x2d,
  0x6e,0x22,0xef,0x3f,0x37,0x04,0xb2,0x6f,0xb7,0xe7,0xd8,0x58,0x05,0xde,0x34,0xbf,
  0x99,0xe6,0x40,0x7a,0x56,0xa7,0x73,0xf5,0x98,0xcb,0xb0,0x37,0x90,0x5e,0xd1,0x3f,
  0xf4,0x73,0x50,0x7f,0x53,0x8e,0xf1,0x04,0x25,0xb4,0x77,0x22,0x4e,0x8a,0x9d,0x27,
  0x8f,0x6f,0xaf,0x59,0xbd,0xb0,0x0f,0xf0,0xaa,0x11,0x94,0x66,0x16,0x10,0x58,0xad,
  0x77,0xa1,0xac,0x58,0xb4,0xd0,0x0d,0xbc,0x11,0xe0,0xc0,0xe9,0x29,0xdc,0x42,0x63,
  0x01,0x23,0x4f,0x28,0x41,0x6d,0x34,0x9e,0x0c,0x4a,0xc8,0x62,0x83,0xb5,0x71,0x71,
  0x0b,0x51,0xc0,0x4c,0x37,0xd4,0x68,0x19,0x52,0x9a,0x8b,0x02,0x03,0x01,0x00,0x01,
  0x02,0x81,0x81,0x00,0x82,0xca,0x33,0x16,0xb2,0x3a,0xd4,0x1b,0x62,0x9a,0x9c,0xc5,
  0x07,0x4f,0x57,0x89,0x2f,0x7c,0x4a,0xdf,0xb4,0x3b,0xc7,0xa4,0x11,0x14,0x2d,0xf4,
  0x4c,0xca,0xcc,0x03,0x88,0x06,0x82,0x34,0xab,0xe7,0xe4,0x24,0x15,0x33,0x1c,0xcb,
  0x0a,0xcf,0xc3,0x27,0x78,0x33,0x6b,0x6f,0x82,0x3e,0x3c,0x70,0xc9,0xe2,0xb9,0x7f,
  0x88,0xc3,0x4f,0x59,0xb5,0x8e,0xa3,0x81,0xd9,0x88,0x1f,0xc0,0x38,0xbc,0xc8,0x93,
  0x40,0x0f,0x43,0xd8,0x72,0x12,0xb4,0xcc,0x6d,0x76,0x0a,0x6f,0x01,0x05,0xa8,0x88,
  0xf4,0x57,0x44,0xd2,0x05,0xc4,0x77,0xf5,0xfb,0x1b,0xf3,0xb2,0x0d,0x90,0xb8,0xb4,
  0x63,0x62,0x70,0x2c,0xe4,0x28,0xd8,0x20,0x10,0x85,0x4a,0x5e,0x63,0xa9,0xb0,0xdd,
  0xba,0xd0,0x32,0x49,0x02,0x41,0x00,0xdb,0x77,0xf1,0xdd,0x1a,0x12,0xc5,0xfb,0x2b,
  0x5b,0xb2,0xcd,0xb6,0xd0,0x4c,0xc4,0xe5,0x93,0xd6,0xf8,0x88,0xfc,0x18,0x40,0x21,
  0x9c,0xf7,0x2d,0x60,0x6f,0x91,0xf5,0x73,0x3c,0xf7,0x7f,0x67,0x1d,0x5b,0xb5,0xee,
  0x29,0xc1,0xd4,0xc6,0xdb,0x44,0x4c,0x40,0x05,0x63,0xaa,0x71,0x95,0x18,0x14,0xa7,
  0x23,0x9f,0x7a,0xee,0x7f,0xb5,0xc7,0x02,0x41,0x00,0xcf,0x2c,0x24,0x50,0x65,0xf4,
  0x94,0x7b,0xe9,0xf3,0x13,0x77,0xea,0x27,0x3c,0x6f,0x03,0x84,0xa7,0x7d,0xa2,0x54,
  0x40,0x97,0x82,0x0e,0xd9,0x09,0x9f,0x4a,0xa6,0x75,0xe5,0x66,0xe4,0x9c,0x59,0xd9,
  0x3a,0xe6,0xf7,0xd8,0x8b,0x68,0xb0,0x21,0x52,0x31,0xb3,0x4a,0xa0,0x2c,0x41,0xd7,
  0x1f,0x7b,0xe2,0x0f,0x15,0xc9,0x6e,0xc0,0xe5,0x1d,0x02,0x41,0x00,0x9c,0x1a,0x61,
  0x9f,0x89,0xc7,0x26,0xa9,0x33,0xba,0xe2,0xa0,0x6d,0xd3,0x15,0x77,0xcb,0x6f,0xef,
  0xad,0x12,0x0a,0x75,0xd9,0x4f,0xcf,0x4d,0x05,0x2a,0x9d,0xd1,0x2c,0xcb,0xcd,0xe6,
  0xa0,0xe9,0x20,0x39,0xb6,0x5a,0xf3,0xba,0x99,0xf4,0xe3,0xcb,0x5d,0x8d,0x00,0x08,
  0x57,0x18,0xb9,0x1a,0xca,0xbd,0xe3,0x99,0xb1,0x1f,0xe9,0x18,0xcb,0x02,0x40,0x65,
  0x35,0x1b,0x48,0x6b,0x86,0x60,0x43,0x68,0xb6,0xe6,0xfb,0xdd,0xd7,0xed,0x1e,0x0e,
  0x89,0xef,0x88,0xe0,0x94,0x68,0x39,0x9b,0xbf,0xc5,0x27,0x7e,0x39,0xe9,0xb8,0x0e,
  0xa9,0x85,0x65,0x1c,0x3f,0x93,0x16,0xe2,0x5d,0x57,0x3d,0x7d,0x4d,0xc9,0xe9,0x9d,
  0xbd,0x07,0x22,0x97,0xc7,0x90,0x09,0xe5,0x15,0x99,0x7f,0x1e,0x2b,0xfd,0xc1,0x02,
  0x41,0x00,0x92,0x78,0xfe,0x04,0xa0,0x53,0xed,0x36,0x97,0xbd,0x16,0xce,0x91,0x9b,
  0xbe,0x1f,0x8e,0x40,0x00,0x99,0x0c,0x49,0x15,0xca,0x59,0xd3,0xe3,0xd4,0xeb,0x71,
  0xcf,0xda,0xd7,0xc8,0x99,0x74,0xfc,0x6b,0xe8,0xfd,0xe5,0xe0,0x49,0x61,0xcb,0xda,
  0xe3,0xe7,0x8b,0x72,0xb5,0x69,0x73,0x2b,0x8b,0x54,0xcb,0xd9,0x48,0x6d,0x61,0x02,
  0x49,0xe8,
};

/**
 * Issue a certificate with a given serial number
 */
static certificate_t* create_cert(chunk_t serial)
{
	private_key_t *privkey;
	public_key_t *pubkey;
	certificate_t *cert;
	identification_t *id;

	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
								 BUILD_BLOB_ASN1_DER, chunk_from_thing(keydata),
								 BUILD_END);
	ck_assert(privkey);
	pubkey = privkey->get_public_key(privkey);
	ck_assert(pubkey);
	id = identification_create_from_string("C=CH, O=strongSwan, CN=test");
	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
						BUILD_SIGNING_KEY, privkey,
						BUILD_PUBLIC_KEY, pubkey,
						BUILD_SUBJECT, id,
						BUILD_SERIAL, serial,
						BUILD_END);
	ck_assert(cert);
	id->destroy(id);
	privkey->destroy(privkey);
	pubkey->destroy(pubkey);

	return cert;
}

CALLBACK(filter, bool,
	void *data, enumerator_t *orig, va_list args)
{
	crl_revoked_t *revoked;
	crl_reason_t *reason;
	chunk_t *serial;
	time_t *date;

	VA_ARGS_VGET(args, serial, date, reason);

	if (orig->enumerate(orig, &revoked))
	{
		*serial = revoked->serial;
		*date   = revoked->date;
		*reason = revoked->reason;
		return TRUE;
	}
	return FALSE;
}

/**
 * Revoke a certificate with a given serial number
 */
static certificate_t* create_crl(chunk_t serial, certificate_t *cert)
{
	private_key_t *privkey;
	certificate_t *crl;
	linked_list_t *list;
	enumerator_t *enumerator;
	crl_revoked_t *revoked;
	time_t date = time(NULL);

	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
								 BUILD_BLOB_ASN1_DER, chunk_from_thing(keydata),
								 BUILD_END);
	ck_assert(privkey);

	INIT(revoked,
		.serial = serial,
		.reason = CRL_REASON_KEY_COMPROMISE,
		.date = date
	);
	list = linked_list_create();
	list->insert_last(list, revoked);

	enumerator = enumerator_create_filter(list->create_enumerator(list),
										  filter, NULL, NULL);
	crl = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_CRL,
						BUILD_SIGNING_KEY, privkey,
						BUILD_NOT_BEFORE_TIME, date,
						BUILD_NOT_AFTER_TIME, date + 30 * 24 * 3600,
						BUILD_SIGNING_CERT, cert,
						BUILD_REVOKED_ENUMERATOR, enumerator,
						BUILD_SERIAL, serial,
						BUILD_BASE_CRL, serial,
						BUILD_END);
	ck_assert(crl);
	enumerator->destroy(enumerator);
	list->destroy(list);
	free(revoked);
	privkey->destroy(privkey);

	return crl;
}

/**
 * Create an OCSP request for a given serial number
 */
static certificate_t* create_ocsp_request(certificate_t *cert)
{
	certificate_t *ocsp_req;

	ocsp_req = lib->creds->create(lib->creds,
						CRED_CERTIFICATE, CERT_X509_OCSP_REQUEST,
						BUILD_CA_CERT, cert,
						BUILD_CERT, cert,
						BUILD_END);

	ck_assert(ocsp_req);

	return ocsp_req;
}

/**
 * Parse an ASN.1 encoded OCSP request
 */
static certificate_t* parse_ocsp_request(chunk_t encoding)
{
	certificate_t *ocsp_req;

	ocsp_req = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_OCSP_REQUEST,
						BUILD_BLOB_ASN1_DER, encoding,
						BUILD_END);
	ck_assert(ocsp_req);

	return ocsp_req;
}

/**
 * Create an OCSP response based on an OCSP request
 */
static certificate_t* create_ocsp_response(ocsp_request_t *ocsp_request,
										   cert_validation_t status,
										   certificate_t *cert)
{
	private_key_t *privkey;
	certificate_t *ocsp_rsp;
	ocsp_status_t ocsp_status = OCSP_SUCCESSFUL;
	ocsp_single_response_t *response;
	chunk_t issuerNameHash, issuerKeyHash, serialNumber, nonce;
	hash_algorithm_t hashAlgorithm;
	linked_list_t *responses = linked_list_create();
	enumerator_t *enumerator;

	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
								 BUILD_BLOB_ASN1_DER, chunk_from_thing(keydata),
								 BUILD_END);
	ck_assert(privkey);

	/* generate OCSP single response */
	enumerator = ocsp_request->create_request_enumerator(ocsp_request);
	ck_assert(enumerator->enumerate(enumerator, &hashAlgorithm, &issuerNameHash,
												&issuerKeyHash, &serialNumber));
	response = ocsp_single_response_create();
	response->hashAlgorithm  = hashAlgorithm;
	response->issuerNameHash = chunk_clone(issuerNameHash);
	response->issuerKeyHash  = chunk_clone(issuerKeyHash);
	response->serialNumber   = chunk_clone(serialNumber);
	response->thisUpdate     = time(NULL);
	response->status         = status;
	if (status == VALIDATION_REVOKED)
	{
		response->revocationReason = CRL_REASON_KEY_COMPROMISE;
		response->revocationTime   = time(NULL);
	}
	responses->insert_last(responses, response);
	enumerator->destroy(enumerator);

	nonce = ocsp_request->get_nonce(ocsp_request);

	/* generate OCSP response */
	enumerator = responses->create_enumerator(responses);
	ocsp_rsp = lib->creds->create(lib->creds, CRED_CERTIFICATE,
						CERT_X509_OCSP_RESPONSE,
						BUILD_OCSP_STATUS, ocsp_status,
						BUILD_OCSP_RESPONSES, enumerator,
						BUILD_SIGNING_KEY, privkey,
						BUILD_SIGNING_CERT, cert,
						BUILD_NONCE, nonce,
						BUILD_END);
	enumerator->destroy(enumerator);

	ck_assert(ocsp_rsp);

	privkey->destroy(privkey);
	responses->destroy_offset(responses, offsetof(ocsp_single_response_t, destroy));

	return ocsp_rsp;
}

/**
 * Parse an ASN.1 encoded OCSP response
 */
static certificate_t* parse_ocsp_response(chunk_t encoding)
{
	certificate_t *ocsp_rsp;

	ocsp_rsp = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_OCSP_RESPONSE,
						BUILD_BLOB_ASN1_DER, encoding,
						BUILD_END);
	ck_assert(ocsp_rsp);

	return ocsp_rsp;
}

/**
 * Issue an attribute certificate with a given serial number
 */
static certificate_t* create_acert(chunk_t serial, certificate_t *cert)
{
	private_key_t *privkey;
	public_key_t *pubkey;
	certificate_t *acert;
	linked_list_t *groups = linked_list_create();
	time_t date = time(NULL);

	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
								 BUILD_BLOB_ASN1_DER, chunk_from_thing(keydata),
								 BUILD_END);
	ck_assert(privkey);
	pubkey = privkey->get_public_key(privkey);
	ck_assert(pubkey);
	groups->insert_last(groups, "research");
	acert = lib->creds->create(lib->creds,
						CRED_CERTIFICATE, CERT_X509_AC,
						BUILD_NOT_BEFORE_TIME, date,
						BUILD_NOT_AFTER_TIME, date + 30 * 24 * 3600,
						BUILD_CERT, cert,
						BUILD_SERIAL, serial,
						BUILD_AC_GROUP_STRINGS, groups,
						BUILD_SIGNING_CERT, cert,
						BUILD_SIGNING_KEY, privkey,
						BUILD_END);
	ck_assert(acert);
	groups->destroy(groups);
	privkey->destroy(privkey);
	pubkey->destroy(pubkey);

	return acert;
}

/**
 * Parse an ASN.1 encoded attribute certificate
 */
static certificate_t* parse_acert(chunk_t encoding)
{
	certificate_t *acert;

	acert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_AC,
						BUILD_BLOB_ASN1_DER, encoding,
						BUILD_END);
	ck_assert(acert);

	return acert;
}

typedef struct {
	chunk_t serial;
	chunk_t serial_asn1;
} serial_number_t;

static serial_number_t serial_numbers[] = {
	{ chunk_from_chars(0x00),
	  chunk_from_chars(0x01,0x00) },
	{ chunk_from_chars(0x01),
	  chunk_from_chars(0x01,0x01) },
	{ chunk_from_chars(0x7f),
	  chunk_from_chars(0x01,0x7f) },
	{ chunk_from_chars(0x80),
	  chunk_from_chars(0x02,0x00,0x80) },
	{ chunk_from_chars(0xff),
	  chunk_from_chars(0x02,0x00,0xff) },
	{ chunk_from_chars(0x01,0x00),
	  chunk_from_chars(0x02,0x01,0x00) },
	{ chunk_from_chars(0x7f,0xff),
	  chunk_from_chars(0x02,0x7f,0xff) },
	{ chunk_from_chars(0x80,0x00),
	  chunk_from_chars(0x03,0x00,0x80,0x00) },
	{ chunk_from_chars(0xff,0xff),
	  chunk_from_chars(0x03,0x00,0xff,0xff) },
	{ chunk_from_chars(0x01,0x00,0x00),
	  chunk_from_chars(0x03,0x01,0x00,0x00) },
	{ chunk_from_chars(0x7f,0xff,0xff),
	  chunk_from_chars(0x03,0x7f,0xff,0xff) },
	{ chunk_from_chars(0x80,0x00,0x00),
	  chunk_from_chars(0x04,0x00,0x80,0x00,0x00) },
	{ chunk_from_chars(0xff,0xff,0xff),
	  chunk_from_chars(0x04,0x00,0xff,0xff,0xff) },
	{ chunk_from_chars(0x01,0x00,0x00,0x00),
	  chunk_from_chars(0x04,0x01,0x00,0x00,0x00) },
	{ chunk_from_chars(0x7f,0xff,0xff,0xff),
	  chunk_from_chars(0x04,0x7f,0xff,0xff,0xff) },
	{ chunk_from_chars(0x80,0x00,0x00,0x00),
	  chunk_from_chars(0x05,0x00,0x80,0x00,0x00,0x00) },
	{ chunk_from_chars(0xff,0xff,0xff,0xff),
	  chunk_from_chars(0x05,0x00,0xff,0xff,0xff,0xff) },
};


START_TEST(test_gen_serial_numbers)
{
	chunk_t encoding, serial, serial_asn1;
	certificate_t *cert, *crl, *ocsp_req, *ocsp_rsp, *acert, *acert1;
	time_t revocation_time, this_update, next_update;
	ocsp_request_t *ocsp_request;
	ocsp_response_t *ocsp_resp;
	cert_validation_t status;
	crl_reason_t revocation_reason;
	crl_t *x509_crl;
	x509_t *x509;
	ac_t *ac;
	size_t offset;
	u_char *pos;
	enumerator_t *enumerator;

	/**
	 * Use serial number with canonical encoding (no leading zeroes)
	 */

	/* create X.509 certificate */
	cert = create_cert(serial_numbers[_i].serial);

	/* retrieve configured serial number */
	x509 = (x509_t*)cert;
	ck_assert_chunk_eq(x509->get_serial(x509), serial_numbers[_i].serial);

	/* the ASN.1 TLV (Type-Length-Value) encoding of an X.509 certificate is
	 *
	 * 0 "x509",             ASN1_SEQUENCE
	 * 1   "tbsCertificate", ASN1_SEQUENCE
	 * 2     "DEFAULT v1",   ASN1_CONTEXT_C_0
	 * 3       "version",    ASN1_INTEGER
	 * 4     "serialNumber", ASN1_INTEGER
	 * ...
	 *
	 * The one octet length field of the serialNumber (4) is at
	 *   pos = 4 (TL0) + 4 (TL1) + 2 (TL2) + 3 (TLV3) + 1 (T4) = 14
	 */
	ck_assert(cert->get_encoding(cert, CERT_ASN1_DER, &encoding));
	DBG2(DBG_LIB, "cert: %B", &encoding);

	/* check ASN.1 integer encoding of serial number */
	pos = encoding.ptr + 14;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);
	chunk_free(&encoding);

	/* create X.509 crl */
	crl = create_crl(serial_numbers[_i].serial, cert);

	/* retrieve configured serial number */
	x509_crl = (crl_t*)crl;
	ck_assert_chunk_eq(x509_crl->get_serial(x509_crl), serial_numbers[_i].serial);

	enumerator = x509_crl->create_enumerator(x509_crl);
	ck_assert(enumerator->enumerate(enumerator, &serial, NULL, NULL));
	ck_assert_chunk_eq(serial, serial_numbers[_i].serial);
	enumerator->destroy(enumerator);

	/* retrieve configured base crl number */
	ck_assert(x509_crl->is_delta_crl(x509_crl, &serial));
	ck_assert_chunk_eq(x509_crl->get_serial(x509_crl), serial_numbers[_i].serial);

	/* the ASN.1 TLV (Type-Length-Value) encoding of an X.509 crl is
	 *
	 *  0 "certificateList",                    ASN1_SEQUENCE
	 *  1   "tbsCertList",                      ASN1_SEQUENCE
	 *  2     "version",                        ASN1_INTEGER
	 *  3     "signature",                      ASN1_SEQUENCE
	 *  4     "issuer",                         ASN1_SEQUENCE
	 *  5     "thisUpdate"                      ASN1_UTCTIME
	 *  6     "nextUpdate"                      ASN1_UTCTIME
	 *  7     "revokedCertificates",            ASN1_SEQUENCE
	 *  8       "certList",                     ASN1_SEQUENCE
	 *  9         "userCertificate",            ASN1_INTEGER
	 * 10         "revocationDate",             ASN1_UTCTIME
	 * 11         "crlEntryExtensions",         ASN1_SEQUENCE
	 * 12     "optional extensions",            ASN1_CONTEXT_C_0
	 * 13       "crlExtensions",                ASN1_SEQUENCE
	 * 14         "extension",                  ASN1_SEQUENCE
	 * 15           "extnID",                   ASN1_OID
	 * 16           "critical",                 ASN1_BOOLEAN
	 * 17           "extnValue",                ASN1_OCTET_STRING
	 * 18             "authorityKeyIdentifier", ASN1_SEQUENCE
	 * 19         "extension",                  ASN1_SEQUENCE
	 * 20           "extnID",                   ASN1_OID
	 * 21           "critical",                 ASN1_BOOLEAN
	 * 22           "extnValue",                ASN1_OCTET_STRING
	 * 23             "crlNumber"               ASN1_INTEGER
	 * 24         "extension",                  ASN1_SEQUENCE
	 * 25           "extnID",                   ASN1_OID
	 * 26           "critical",                 ASN1_BOOLEAN
	 * 27           "extnValue",                ASN1_OCTET_STRING
	 * 28             "baseCrlNumber"           ASN1_INTEGER
	 * ...
	 *
	 * The one octet length field of the revoked userCertificate (9) is at
	 *   pos = 4 (TL0) + 3 (TL1) + 3 (TLV2) + 15 (TLV3) + 51 (TLV4) + 15 (TLV5) +
	 *         15 (TLV6) + 2 (TL7) + 2 (TL8) + 1 (T9) = 111
	 *
	 * The one octet length field of the crlNumber extension (19) is at
	 *   offset = pos - 1 (T9) + *L8 + 2 (TL12) + 2 (TL13) + 33 (TLV14) + 1 (T19)
	 *          = 110 + *L8 + 38
	 *
	 * The one octet length field of the crlNumber (23) is at
	 *   pos = offset + 1 (L19) + 5 (TLV20) + 0 (TLV21) + 2 (TL22) + 1 (T23)
	 *       = offset + 9
	 *
	 * The one octet length field of the baseCrlNumber (28) is at
	 *   pos = offset + 1 (L19) + *L19 + 2 (TL24) + 5 (TLV25) + 3 (TLV26)
	 *                + 2 (TL27) + 1 (T28)
	 *       = offset + *L19 + 14
	 */
	ck_assert(crl->get_encoding(crl, CERT_ASN1_DER, &encoding));
	DBG2(DBG_LIB, "crl: %B", &encoding);

	/* check ASN.1 integer encoding of revoked number */
	pos = encoding.ptr + 111;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);

	/* check ASN.1 integer encoding of crlNumber */
	offset = 110 + encoding.ptr[109] + 38;
	pos = encoding.ptr + offset + 9;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);

	/* check ASN.1 integer encoding of baseCrlNumber */
	pos = encoding.ptr + offset + encoding.ptr[offset] + 14;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);
	chunk_free(&encoding);

	/* create ocsp request */
	ocsp_req = create_ocsp_request(cert);

	/* the ASN.1 TLV (Type-Length-Value) encoding of an OCSP request is
	 *
	 * 0 "OCSPRequest",              ASN1_SEQUENCE
	 * 1   "tbsRequest",             ASN1_SEQUENCE
	 * 2     "requestList",          ASN1_SEQUENCE
	 * 3       "request",            ASN1_SEQUENCE
	 * 4         "reqCert",          ASN1_SEQUENCE,
	 * 5           "hashAlgorithm",  ASN1_SEQUENCE
	 * 6           "issuerNameHash", ASN1_OCTET STRING
	 * 7           "issuerKeyHash",  ASN1_OCTET STRING
	 * 8           "serialNumber",   ASN1_INTEGER
	 * ...
	 *
	 * The one octet length field of the serialNumber (8) is at
	 * pos = 3 (TL0) + 3 (TL1) + 2 (TL2) + 2 (TL3) + 2 (TL4) + 11 (TLV5) +
	         22 (TLV6) + 22 (TLV7) + 1 (T8) = 68
	 */
	ck_assert(ocsp_req->get_encoding(ocsp_req, CERT_ASN1_DER, &encoding));
	DBG2(DBG_LIB, "ocsp request: %B", &encoding);

	/* check ASN.1 integer encoding of requested serial number */
	pos = encoding.ptr + 68;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);
	ocsp_req->destroy(ocsp_req);

	/* parse ocsp request */
	ocsp_req = parse_ocsp_request(encoding);
	ocsp_request = (ocsp_request_t*)ocsp_req;

	/* test ocsp request */
	enumerator = ocsp_request->create_request_enumerator(ocsp_request);
	ck_assert(enumerator->enumerate(enumerator, NULL, NULL, NULL, &serial));
	ck_assert_chunk_eq(serial, serial_numbers[_i].serial);
	enumerator->destroy(enumerator);
	chunk_free(&encoding);

	/* create ocsp response */
	status = (_i % 2) ? VALIDATION_GOOD : VALIDATION_REVOKED;
	ocsp_rsp = create_ocsp_response(ocsp_request, status, cert);

	/* the ASN.1 TLV (Type-Length-Value) encoding of an OCSP response is
	 *
	 * 0 "OCSPResponse",                     ASN1_SEQUENCE
	 * 1   "responseStatus",                 ASN1_ENUMERATED
	 * 1   "responseBytesContext",           ASN1_CONTEXT_C_0
	 * 2     "responseBytes",                ASN1_SEQUENCE
	 * 3       "responseType",               ASN1_OID
	 * 3       "response",                   ASN1_OCTET_STRING
	 * 4         "BasicOCSPResponse",        ASN1_SEQUENCE
	 * 5           "tbsResponseData",        ASN1_SEQUENCE
	 * 6             "responderIdContext",   ASN1_CONTEXT_C_1
	 * 6             "producedAt",           ASN1_GENERALIZEDTIME
	 * 6             "responses",            ASN1_SEQUENCE
	 * 7               "singleResponse",     ASN1_SEQUENCE
	 * 8                 "certID",           ASN1_SEQUENCE
	 * 9                   "algorithm",      ASN1_SEQUENCE
	 * 9                   "issuerNameHash", ASN1_OCTET_STRING
	 * 9                   "issuerKeyHash",  ASN1_OCTET_STRING
	 * 9                   "serialNumber",   ASN1_INTEGER
	 *
	 * The one octet length field of the serialNumber (8) is at
     * pos = 4 (TL0) + 3 (TLV1) + 4 (TL1) + 4 (TL2) + 11 (TLV3) + 4 (TL3) +
             4 (TL4) + 3 (TL5) + 53 (TLV6) + 17 (TVL6) + 2 (TL6) + 2 (TL7) +
             2 (TL8) + 11 (TLV9) + 22 (TLV9) + 22 (TLV9) + 1 (T9) = 169
     */
	ck_assert(ocsp_rsp->get_encoding(ocsp_rsp, CERT_ASN1_DER, &encoding));
	DBG2(DBG_LIB, "ocsp response: %B", &encoding);

	/* check ASN.1 integer encoding of requested serial number */
	pos = encoding.ptr + 169;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);
	ocsp_rsp->destroy(ocsp_rsp);

	/* parse ocsp response */
	ocsp_rsp = parse_ocsp_response(encoding);
	ocsp_resp = (ocsp_response_t*)ocsp_rsp;

	/* test ocsp response */
	ck_assert_chunk_eq(ocsp_request->get_nonce(ocsp_request),
					   ocsp_resp->get_nonce(ocsp_resp));

	status = ocsp_resp->get_status(ocsp_resp, x509, x509, &revocation_time,
								&revocation_reason, &this_update, &next_update);
	if (_i % 2)
	{
		ck_assert(status == VALIDATION_GOOD);
	}
	else
	{
		ck_assert(status == VALIDATION_REVOKED);
		ck_assert(revocation_reason == CRL_REASON_KEY_COMPROMISE);
	}

	enumerator = ocsp_resp->create_response_enumerator(ocsp_resp);
	ck_assert(enumerator->enumerate(enumerator, &serial, &status,
									&revocation_time, &revocation_reason));
	ck_assert_chunk_eq(serial, serial_numbers[_i].serial);
	if (_i % 2)
	{
		ck_assert(status == VALIDATION_GOOD);
	}
	else
	{
		ck_assert(status == VALIDATION_REVOKED);
		ck_assert(revocation_reason == CRL_REASON_KEY_COMPROMISE);
	}
	enumerator->destroy(enumerator);
	chunk_free(&encoding);

	/* create attribute certificate */
	acert = create_acert(serial_numbers[_i].serial, cert);

	/* retrieve configured serial number */
	ac = (ac_t*)acert;
	ck_assert_chunk_eq(ac->get_serial(ac), serial_numbers[_i].serial);

	/* retrieve configured holderSerial number */
	ck_assert_chunk_eq(ac->get_holderSerial(ac), serial_numbers[_i].serial);

	/* the ASN.1 TLV (Type-Length-Value) encoding of an attribute certificate is
	 *
	 *  0 "AttributeCertificate",                  ASN1_SEQUENCE
	 *  1   "AttributeCertificateInfo",            ASN1_SEQUENCE
	 *  2     "version",                           ASN1_INTEGER
	 *  3     "holder",                            ASN1_SEQUENCE
	 *  4      "baseCertificateID",                ASN1_CONTEXT_C_0
	 *  5         "issuer",                        ASN1_SEQUENCE
	 *  6         "serial",                        ASN1_INTEGER
	 *  7         "issuerUID",                     ASN1_BIT_STRING
	 *  8       "entityName",                      ASN1_CONTEXT_C_1
	 *  9       "objectDigestInfo",                ASN1_CONTEXT_C_2
	 * 10     "v2Form",                            ASN1_CONTEXT_C_0
	 * 11     "signature",                         ASN1_SEQUENCE
	 * 12     "serialNumber"                       ASN1_INTEGER
	 * 13     "attrCertValidityPeriod",            ASN1_SEQUENCE
	 * 14     "attributes",                        ASN1_SEQUENCE
	 * 15     "extensions",                        ASN1_SEQUENCE
	 * 16       "extension",                       ASN1_SEQUENCE
	 * 17         "extnID",                        ASN1_OID
	 * 18         "critical",                      ASN1_BOOLEAN
	 * 19         "extnValue",                     ASN1_OCTET_STRING
	 * 20           "authorityKeyIdentifier",      ASN1_SEQUENCE
	 * 21             "keyIdentifier",             ASN1_CONTEXT_S_0
	 * 22             "authorityCertIssuer",       ASN1_CONTEXT_C_1
	 * 23             "authorityCertSerialNumber", ASN1_CONTEXT_S_2
	 * ...
	 *
	 * The one octet length field of the holder serial number (6) is at
	 *   pos = 4 (TL0) + 4 (TL1) + 3 (TLV2) + 2 (TL3) + 2 (TL4) + 55 (TLV5) +
	 *         1 (T6) = 71
	 *
	 * The one octet length field of the serialNumber (12) is at
	 *   offset = 4 (TL0) + 4 (TL1) + 3 (TLV2) + 2 (TL3) + *L3 + 57 (TLV10) +
	 *            15 (TLV11) + 1 (T12)
	 *          = 13 + *L3 + 73
	 *
	 * The one octet length field of the authorityCertSerialNumber (23) is at
	 *   pos = offset + *L12 + 1 (L12) + 36 (TLV13) + 30 (TLV14) + 2 (TL15)
	 *                + 2 (TL16) + 5 (TLV17) + 0 (TLV18) + 2 (TL19) + 2 (TL20)
	 *                + 22 (TLV21) + 55 (TLV22) + 1 (T23)
	 *   pos = offset + *L12 + 158
	 */
	ck_assert(acert->get_encoding(acert, CERT_ASN1_DER, &encoding));
	DBG2(DBG_LIB, "acert: %B", &encoding);

	/* check ASN.1 integer encoding of holder serial number */
	pos = encoding.ptr + 71;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);

	/* check ASN.1 integer encoding of the AC serialNumber */
	offset = 13 + encoding.ptr[12] + 73;
	pos = encoding.ptr + offset;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);

	/* check ASN.1 integer encoding of serial number */
	pos = encoding.ptr + offset + encoding.ptr[offset] + 158;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);

	/* parse ASN.1 encoded attribute certificate */
	acert1 = parse_acert(encoding);
	ac = (ac_t*)acert1;

	/* check serial number */
	ck_assert_chunk_eq(ac->get_serial(ac), serial_numbers[_i].serial);

	/* check holderSerial number */
	ck_assert_chunk_eq(ac->get_holderSerial(ac), serial_numbers[_i].serial);
	chunk_free(&encoding);

	cert->destroy(cert);
	crl->destroy(crl);
	ocsp_req->destroy(ocsp_req);
	ocsp_rsp->destroy(ocsp_rsp);
	acert->destroy(acert);
	acert1->destroy(acert1);

	/**
	 * Use serial number with two's complement encoding
	 */

	serial = chunk_skip(serial_numbers[_i].serial_asn1, 1);

	/* create certificate */
	cert = create_cert(serial);

	/* retrieve configured serial number */
	x509 = (x509_t*)cert;
	ck_assert_chunk_eq(x509->get_serial(x509), serial_numbers[_i].serial);

	/* check ASN.1 integer encoding */
	ck_assert(cert->get_encoding(cert, CERT_ASN1_DER, &encoding));
	pos = encoding.ptr + 14;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);
	chunk_free(&encoding);

	/* create crl */
	crl = create_crl(serial, cert);

	/* retrieve configured serial number */
	x509_crl = (crl_t*)crl;
	ck_assert_chunk_eq(x509_crl->get_serial(x509_crl), serial_numbers[_i].serial);

	enumerator = x509_crl->create_enumerator(x509_crl);
	if (enumerator->enumerate(enumerator, &serial, NULL, NULL))
	{
		ck_assert_chunk_eq(serial, serial_numbers[_i].serial);
	}
	enumerator->destroy(enumerator);

	/* check ASN.1 integer encoding of revoked number */
	ck_assert(crl->get_encoding(crl, CERT_ASN1_DER, &encoding));
	pos = encoding.ptr + 111;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);

	/* check ASN.1 integer encoding of crlNumber */
	offset = 110 + encoding.ptr[109] + 38;
	pos = encoding.ptr + offset + 9;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);

	/* check ASN.1 integer encoding of baseCrlNumber */
	pos = encoding.ptr + offset + encoding.ptr[offset] + 14;
	serial_asn1 = chunk_create(pos, 1 + *pos);
	ck_assert_chunk_eq(serial_asn1, serial_numbers[_i].serial_asn1);
	chunk_free(&encoding);

	cert->destroy(cert);
	crl->destroy(crl);
}
END_TEST

Suite *serial_gen_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("serial_gen");

	tc = tcase_create("generate serial numbers");
	tcase_add_loop_test(tc, test_gen_serial_numbers, 0, countof(serial_numbers));
	suite_add_tcase(s, tc);

	return s;
}
