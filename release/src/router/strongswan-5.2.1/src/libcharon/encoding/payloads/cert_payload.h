/*
 * Copyright (C) 2008 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

/**
 * @defgroup cert_payload cert_payload
 * @{ @ingroup payloads
 */

#ifndef CERT_PAYLOAD_H_
#define CERT_PAYLOAD_H_

typedef struct cert_payload_t cert_payload_t;
typedef enum cert_encoding_t cert_encoding_t;

#include <library.h>
#include <credentials/certificates/certificate.h>
#include <credentials/containers/container.h>
#include <encoding/payloads/payload.h>

/**
 * Certificate encodings, as in RFC4306
 */
enum cert_encoding_t {
	ENC_PKCS7_WRAPPED_X509 =		 1,
	ENC_PGP =						 2,
	ENC_DNS_SIGNED_KEY =			 3,
	ENC_X509_SIGNATURE =			 4,
	ENC_KERBEROS_TOKEN	=			 6,
	ENC_CRL =						 7,
	ENC_ARL =						 8,
	ENC_SPKI =						 9,
	ENC_X509_ATTRIBUTE =			10,
	ENC_RAW_RSA_KEY =				11,
	ENC_X509_HASH_AND_URL =			12,
	ENC_X509_HASH_AND_URL_BUNDLE =	13,
	ENC_OCSP_CONTENT =				14,  /* from RFC 4806 */
};

/**
 * Enum names for cert_encoding_t
 */
extern enum_name_t *cert_encoding_names;

/**
 * Class representing an IKEv1/IKEv2 CERT payload.
 */
struct cert_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Get the payloads encoded certificate.
	 *
	 * @return				certificate copy
	 */
	certificate_t *(*get_cert)(cert_payload_t *this);

	/**
	 * Get the payloads certificate container.
	 *
	 * @return				container copy
	 */
	container_t *(*get_container)(cert_payload_t *this);

	/**
	 * Get the encoding of the certificate.
	 *
	 * @return				encoding
	 */
	cert_encoding_t (*get_cert_encoding)(cert_payload_t *this);

	/**
	 * Get the hash if this is a hash and URL encoded certificate.
	 *
	 * This function returns internal data, do not free.
	 *
	 * @return				hash
	 */
	chunk_t (*get_hash)(cert_payload_t *this);

	/**
	 * Get the URL if this is a hash and URL encoded certificate.
	 *
	 * This function returns internal data, do not free.
	 *
	 * @return				url
	 */
	char *(*get_url)(cert_payload_t *this);

	/**
	 * Destroys the cert_payload object.
	 */
	void (*destroy) (cert_payload_t *this);
};

/**
 * Creates an empty certificate payload.
 *
 * @param type				payload type (for IKEv1 or IKEv2)
 * @return					cert_payload_t object
 */
cert_payload_t *cert_payload_create(payload_type_t type);

/**
 * Creates a certificate payload with an embedded certificate.
 *
 * @param type				payload type (for IKEv1 or IKEv2)
 * @param cert				certificate to embed
 * @return					cert_payload_t object
 */
cert_payload_t *cert_payload_create_from_cert(payload_type_t type,
											  certificate_t *cert);

/**
 * Creates an IKEv2 certificate payload with hash and URL encoding.
 *
 * @param hash				hash of the DER encoded certificate (get's cloned)
 * @param url				URL to the certificate
 * @return					cert_payload_t object
 */
cert_payload_t *cert_payload_create_from_hash_and_url(chunk_t hash, char *url);

/**
 * Creates a custom certificate payload using type and associated data.
 *
 * @param type				payload type (for IKEv1 or IKEv2)
 * @param encoding			encoding type of certificate
 * @param data				associated data (gets owned)
 * @return					cert_payload_t object
 */
cert_payload_t *cert_payload_create_custom(payload_type_t type,
										cert_encoding_t encoding, chunk_t data);

#endif /** CERT_PAYLOAD_H_ @}*/
