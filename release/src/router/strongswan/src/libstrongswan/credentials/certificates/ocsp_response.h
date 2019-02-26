/*
 * Copyright (C) 2008 Martin Willi
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

/**
 * @defgroup ocsp_response ocsp_response
 * @{ @ingroup certificates
 */

#ifndef OCSP_RESPONSE_H_
#define OCSP_RESPONSE_H_

#include <credentials/certificates/x509.h>
#include <credentials/certificates/crl.h>

typedef struct ocsp_response_t ocsp_response_t;
typedef enum ocsp_status_t ocsp_status_t;

/**
 * OCSP response status
 */
enum ocsp_status_t {
	OCSP_SUCCESSFUL 		= 0,
	OCSP_MALFORMEDREQUEST	= 1,
	OCSP_INTERNALERROR		= 2,
	OCSP_TRYLATER			= 3,
	OCSP_SIGREQUIRED		= 5,
	OCSP_UNAUTHORIZED		= 6,
};

/**
 * enum names for ocsp_status_t
 */
extern enum_name_t *ocsp_status_names;

/**
 * OCSP response message.
 */
struct ocsp_response_t {

	/**
	 * Implements certificate_t interface
	 */
	certificate_t certificate;

	/**
	 * Check the status of a certificate by this OCSP response.
	 *
	 * @param subject			certificate to check status
	 * @param issuer			issuer certificate of subject
	 * @param revocation_time	receives time of revocation, if revoked
	 * @param revocation_reason	receives reason of revocation, if revoked
	 * @param this_update		creation time of revocation list
	 * @param next_update		exptected time of next revocation list
	 * @return					certificate revocation status
	 */
	cert_validation_t (*get_status)(ocsp_response_t *this,
									x509_t *subject, x509_t *issuer,
									time_t *revocation_time,
									crl_reason_t *revocation_reason,
									time_t *this_update, time_t *next_update);

	/**
	 * Create an enumerator over the contained certificates.
	 *
	 * @return					enumerator over certificate_t*
	 */
	enumerator_t* (*create_cert_enumerator)(ocsp_response_t *this);

	/**
	 * Create an enumerator over the contained responses.
	 *
	 * @return					enumerator over major response fields
	 */
	enumerator_t* (*create_response_enumerator)(ocsp_response_t *this);
};

#endif /** OCSP_RESPONSE_H_ @}*/
