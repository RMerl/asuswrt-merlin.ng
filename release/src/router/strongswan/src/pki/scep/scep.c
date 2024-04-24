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

#include <string.h>
#include <stdlib.h>

#include <library.h>
#include <utils/debug.h>
#include <utils/lexparser.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <asn1/oid.h>
#include <crypto/rngs/rng.h>
#include <crypto/hashers/hasher.h>

#include "scep.h"

static const char *operations[] = {
	"PKIOperation",
	"GetCACert",
	"GetCACaps"
};

static const char *pkiStatus_values[] = { "0", "2", "3" };

static const char *pkiStatus_names[] DBG_UNUSED = {
	"SUCCESS",
	"FAILURE",
	"PENDING",
	"UNKNOWN"
};

static const char *msgType_values[] = { "3", "17", "19", "20", "21", "22" };

static const char *msgType_names[] DBG_UNUSED = {
	"CertRep",
	"RenewalReq",
	"PKCSReq",
	"CertPoll",
	"GetCert",
	"GetCRL",
	"Unknown"
};

static const char *failInfo_reasons[] DBG_UNUSED = {
	"badAlg - unrecognized or unsupported algorithm identifier",
	"badMessageCheck - integrity check failed",
	"badRequest - transaction not permitted or supported",
	"badTime - Message time field was not sufficiently close to the system time",
	"badCertId - No certificate could be identified matching the provided criteria"
};

static const char *caps_names[] = {
	"AES",
	"DES3",
	"SHA-256",
	"SHA-384",
	"SHA-512",
	"SHA-224",
	"SHA-1",
	"POSTPKIOperation",
	"SCEPStandard",
	"GetNextCACert",
	"Renewal"
};

const scep_attributes_t empty_scep_attributes = {
	SCEP_Unknown_MSG   , /* msgType */
	SCEP_UNKNOWN       , /* pkiStatus */
	SCEP_unknown_REASON, /* failInfo */
	{ NULL, 0 }        , /* transID */
	{ NULL, 0 }        , /* senderNonce */
	{ NULL, 0 }        , /* recipientNonce */
};

/**
 * Parse CA Capabilities of SCEP server
 */
uint32_t scep_parse_caps(chunk_t response)
{
	uint32_t caps_flags = 0;
	chunk_t line;

	DBG2(DBG_APP, "CA Capabilities:");

	while (fetchline(&response, &line))
	{
		int i;

		for (i = 0; i < countof(caps_names); i++)
		{
			if (strncaseeq(caps_names[i], line.ptr, line.len))
			{
				DBG2(DBG_APP, "  %s", caps_names[i]);
				caps_flags |= (1 << i);
			}
		}
	}
	return caps_flags;
}

/**
 * Generate a transaction ID as the SHA-1 hash of the publicKeyInfo
 * The transaction ID is also used as a unique serial number
 */
bool scep_generate_transaction_id(public_key_t *public,
								  chunk_t *transId, chunk_t *serialNumber)
{
	chunk_t digest;
	int zeros = 0, msb_set = 0;

	if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &digest))
	{
		/* the transaction ID is the fingerprint in hex format */
		*transId = chunk_to_hex(digest, NULL, TRUE);

		/**
		 * the serial number must be a valid positive ASN.1 integer
	     * remove leading zeros, add one if MSB is set (two's complement)
	     */
		while (zeros < digest.len)
		{
			if (digest.ptr[zeros])
			{
				if (digest.ptr[zeros] & 0x80)
				{
					msb_set = 1;
				}
				break;
			}
			zeros++;
		}
		*serialNumber = chunk_alloc(digest.len - zeros + msb_set);
		if (msb_set)
		{
			serialNumber->ptr[0] = 0x00;
		}
		memcpy(serialNumber->ptr + msb_set, digest.ptr + zeros,
			   digest.len - zeros);
		return TRUE;
	}
	return FALSE;
}

/**
 * Builds a PKCS#7 enveloped and signed SCEP request
 */
chunk_t scep_build_request(chunk_t data, chunk_t transID, scep_msg_t msg,
					certificate_t *enc_cert, encryption_algorithm_t enc_alg,
					size_t key_size, certificate_t *signer_cert,
					hash_algorithm_t digest_alg, signature_params_t *scheme,
					private_key_t *private_key)
{
	chunk_t request;
	container_t *container;
	char nonce[16];
	rng_t *rng;
	chunk_t senderNonce, msgType;

	/* generate senderNonce */
	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng || !rng->get_bytes(rng, sizeof(nonce), nonce))
	{
		DESTROY_IF(rng);
		return chunk_empty;
	}
	rng->destroy(rng);

	/* encrypt data in enveloped-data PKCS#7 */
	container = lib->creds->create(lib->creds,
					CRED_CONTAINER, CONTAINER_PKCS7_ENVELOPED_DATA,
					BUILD_BLOB, data,
					BUILD_CERT, enc_cert,
					BUILD_ENCRYPTION_ALG, enc_alg,
					BUILD_KEY_SIZE, (int)key_size,
					BUILD_END);
	if (!container)
	{
		return chunk_empty;
	}
	if (!container->get_encoding(container, &request))
	{
		container->destroy(container);
		return chunk_empty;
	}
	container->destroy(container);

	/* sign enveloped-data in a signed-data PKCS#7 */
	senderNonce = asn1_wrap(ASN1_OCTET_STRING, "c", chunk_from_thing(nonce));
	transID = asn1_wrap(ASN1_PRINTABLESTRING, "c", transID);
	msgType = asn1_wrap(ASN1_PRINTABLESTRING, "c",
						chunk_create((char*)msgType_values[msg],
									 strlen(msgType_values[msg])));

	container = lib->creds->create(lib->creds,
					CRED_CONTAINER, CONTAINER_PKCS7_SIGNED_DATA,
					BUILD_BLOB, request,
					BUILD_SIGNING_CERT, signer_cert,
					BUILD_SIGNING_KEY, private_key,
					BUILD_DIGEST_ALG, digest_alg,
					BUILD_SIGNATURE_SCHEME, scheme,
					BUILD_PKCS7_ATTRIBUTE, OID_PKI_SENDER_NONCE, senderNonce,
					BUILD_PKCS7_ATTRIBUTE, OID_PKI_TRANS_ID, transID,
					BUILD_PKCS7_ATTRIBUTE, OID_PKI_MESSAGE_TYPE, msgType,
					BUILD_END);

	free(request.ptr);
	free(senderNonce.ptr);
	free(transID.ptr);
	free(msgType.ptr);

	if (!container)
	{
		return chunk_empty;
	}
	if (!container->get_encoding(container, &request))
	{
		container->destroy(container);
		return chunk_empty;
	}
	container->destroy(container);

	return request;
}

/**
 * Converts a binary request to base64 with 64 characters per line
 * newline and '+' characters are escaped by %0A and %2B, respectively
 */
static char* escape_http_request(chunk_t req)
{
	char *escaped_req = NULL;
	char *p1, *p2;
	int lines = 0;
	int plus  = 0;
	int n     = 0;

	/* compute and allocate the size of the base64-encoded request */
	int len = 1 + 4 * ((req.len + 2) / 3);
	char *encoded_req = malloc(len);

	/* do the base64 conversion */
	chunk_t base64 = chunk_to_base64(req, encoded_req);
	len = base64.len + 1;

	/* compute newline characters to be inserted every 64 characters */
	lines = (len - 2) / 64;

	/* count number of + characters to be escaped */
	p1 = encoded_req;
	while (*p1 != '\0')
	{
		if (*p1++ == '+')
		{
			plus++;
		}
	}

	escaped_req = malloc(len + 3 * (lines + plus));

	/* escape special characters in the request */
	p1 = encoded_req;
	p2 = escaped_req;
	while (*p1 != '\0')
	{
		if (n == 64)
		{
			memcpy(p2, "%0A", 3);
			p2 += 3;
			n = 0;
		}
		if (*p1 == '+')
		{
			memcpy(p2, "%2B", 3);
			p2 += 3;
		}
		else
		{
			*p2++ = *p1;
		}
		p1++;
		n++;
	}
	*p2 = '\0';
	free(encoded_req);
	return escaped_req;
}

/**
 * Send a SCEP request via HTTP and wait for a response
 */
bool scep_http_request(const char *url, scep_op_t op, bool http_post,
					   chunk_t data, chunk_t *response, u_int *http_code)
{
	int len;
	status_t status;
	char *complete_url = NULL;
	const char *operation;
	host_t *srcip = NULL;

	uint32_t http_timeout = lib->settings->get_time(lib->settings,
										"%s.scep.http_timeout", 30, lib->ns);

	char *http_bind = lib->settings->get_str(lib->settings,
										"%s.scep.http_bind", NULL, lib->ns);

	if (http_bind)
	{
		srcip = host_create_from_string(http_bind, 0);
	}
	DBG2(DBG_APP, "sending scep request to '%s'", url);

	/* initialize response */
	*response = chunk_empty;
	*http_code = 0;

	operation = operations[op];
	switch (op)
	{
		case SCEP_PKI_OPERATION:
		default:
			if (http_post)
			{
				/* form complete url */
				len = strlen(url) + 11 + strlen(operation) + 1;
				complete_url = malloc(len);
				snprintf(complete_url, len, "%s?operation=%s", url, operation);

				status = lib->fetcher->fetch(lib->fetcher, complete_url, response,
										 FETCH_TIMEOUT, http_timeout,
										 FETCH_REQUEST_DATA, data,
										 FETCH_REQUEST_TYPE, "",
										 FETCH_REQUEST_HEADER, "Expect:",
										 FETCH_SOURCEIP, srcip,
										 FETCH_RESPONSE_CODE, http_code,
										 FETCH_END);
			}
			else /* HTTP_GET */
			{
				char *msg = escape_http_request(data);

				/* form complete url */
				len = strlen(url) + 20 + strlen(operation) + strlen(msg) + 1;
				complete_url = malloc(len);
				snprintf(complete_url, len, "%s?operation=%s&message=%s"
						, url, operation, msg);
				free(msg);

				status = lib->fetcher->fetch(lib->fetcher, complete_url, response,
										 FETCH_TIMEOUT, http_timeout,
										 FETCH_REQUEST_HEADER, "Pragma:",
										 FETCH_REQUEST_HEADER, "Host:",
										 FETCH_REQUEST_HEADER, "Accept:",
										 FETCH_SOURCEIP, srcip,
										 FETCH_RESPONSE_CODE, http_code,
										 FETCH_END);
			}
			break;
		case SCEP_GET_CA_CERT:
		case SCEP_GET_CA_CAPS:
		{
			/* form complete url */
			len = strlen(url) + 11 + strlen(operation)  + 1;
			complete_url = malloc(len);
			snprintf(complete_url, len, "%s?operation=%s", url, operation);

			status = lib->fetcher->fetch(lib->fetcher, complete_url, response,
									 FETCH_TIMEOUT, http_timeout,
									 FETCH_SOURCEIP, srcip,
									 FETCH_RESPONSE_CODE, http_code,
									 FETCH_END);
		}
	}
	DESTROY_IF(srcip);
	free(complete_url);

	return (status == SUCCESS);
}

/**
 * Extract X.501 attributes
 */
static void extract_attributes(pkcs7_t *pkcs7, enumerator_t *enumerator,
							   scep_attributes_t *attrs)
{
	chunk_t attr;

	if (pkcs7->get_attribute(pkcs7, OID_PKI_MESSAGE_TYPE, enumerator, &attr))
	{
		scep_msg_t m;

		for (m = SCEP_CertRep_MSG; m < SCEP_Unknown_MSG; m++)
		{
			if (strncmp(msgType_values[m], attr.ptr, attr.len) == 0)
			{
				attrs->msgType = m;
			}
		}
		DBG2(DBG_APP, "messageType:  %s", msgType_names[attrs->msgType]);
		free(attr.ptr);
	}
	if (pkcs7->get_attribute(pkcs7, OID_PKI_STATUS, enumerator, &attr))
	{
		pkiStatus_t s;

		for (s = SCEP_SUCCESS; s < SCEP_UNKNOWN; s++)
		{
			if (strncmp(pkiStatus_values[s], attr.ptr, attr.len) == 0)
			{
				attrs->pkiStatus = s;
			}
		}
		DBG2(DBG_APP, "pkiStatus:    %s", pkiStatus_names[attrs->pkiStatus]);
		free(attr.ptr);
	}
	if (pkcs7->get_attribute(pkcs7, OID_PKI_FAIL_INFO, enumerator, &attr))
	{
		if (attr.len == 1 && *attr.ptr >= '0' && *attr.ptr <= '4')
		{
			attrs->failInfo = (failInfo_t)(*attr.ptr - '0');
		}
		if (attrs->failInfo != SCEP_unknown_REASON)
		{
			DBG1(DBG_APP, "failInfo:     %s", failInfo_reasons[attrs->failInfo]);
		}
		free(attr.ptr);
	}

	pkcs7->get_attribute(pkcs7, OID_PKI_SENDER_NONCE, enumerator,
						 &attrs->senderNonce);
	pkcs7->get_attribute(pkcs7, OID_PKI_RECIPIENT_NONCE, enumerator,
						 &attrs->recipientNonce);
	pkcs7->get_attribute(pkcs7, OID_PKI_TRANS_ID, enumerator,
						 &attrs->transID);
}

/**
 * Parse PKCS#7 encoded SCEP response
 */
bool scep_parse_response(chunk_t response, chunk_t transID,
						  container_t **out, scep_attributes_t *attrs)
{
	enumerator_t *enumerator;
	bool verified = FALSE;
	container_t *container;
	auth_cfg_t *auth;

	*out = NULL;

	container = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS7,
								   BUILD_BLOB_ASN1_DER, response, BUILD_END);
	if (!container)
	{
		DBG1(DBG_APP, "error parsing the scep response");
		return FALSE;
	}
	if (container->get_type(container) != CONTAINER_PKCS7_SIGNED_DATA)
	{
		DBG1(DBG_APP, "scep response is not PKCS#7 signed-data");
		goto error;
	}

	enumerator = container->create_signature_enumerator(container);
	while (enumerator->enumerate(enumerator, &auth))
	{
		verified = TRUE;
		extract_attributes((pkcs7_t*)container, enumerator, attrs);
		if (!chunk_equals(transID, attrs->transID))
		{
			enumerator->destroy(enumerator);
			DBG1(DBG_APP, "transaction ID of scep response does not match");
			goto error;
		}
	}
	enumerator->destroy(enumerator);

	if (!verified)
	{
		DBG1(DBG_APP, "unable to verify PKCS#7 container");
		goto error;
	}
	*out = container;

	return TRUE;

error:
	container->destroy(container);
	return FALSE;
}
