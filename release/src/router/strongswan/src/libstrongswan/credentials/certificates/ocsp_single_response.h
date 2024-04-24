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

/**
 * @defgroup ocsp_single_response ocsp_single_response
 * @{ @ingroup certificates
 */

#ifndef OCSP_SINGLE_RESPONSE_H_
#define OCSP_SINGLE_RESPONSE_H_

#include <credentials/certificates/x509.h>
#include <credentials/certificates/crl.h>

typedef struct ocsp_single_response_t ocsp_single_response_t;

/**
 * Single response contained in OCSP response
 */
struct ocsp_single_response_t {

	/**
	 *  Hash algorithm for the two hashes
	 */
	int hashAlgorithm;

	/**
	 *  hash of issuer DN
	 */
	chunk_t issuerNameHash;

	/**
	 * issuerKeyID
	 */
	chunk_t issuerKeyHash;

	/**
	 * Serial number of certificate
	 */
	chunk_t serialNumber;

	/**
	 * OCSP certificate status
	 */
	cert_validation_t status;

	/**
	 * Time of revocation, if revoked
	 */
	time_t revocationTime;

	/**
	 * Revocation reason, if revoked
	 */
	crl_reason_t revocationReason;

	/**
	 * Creation of the OCSP single response
	 */
	time_t thisUpdate;

	/**
	 * Creation of next OCSP single response
	 */
	time_t nextUpdate;

	/**
	 * Get a new reference to the ocsp_single_response object.
	 *
	 * @return			this, with an increased refcount
	 */
	ocsp_single_response_t* (*get_ref)(ocsp_single_response_t *this);

	/**
	 * Destroy an ocsp_single_response_t object.
	 */
	void (*destroy)(ocsp_single_response_t *this);
};

/**
 * Create an ocsp_single_response_t object
 *
 * @return              ocsp_single_response_t object
 */
ocsp_single_response_t *ocsp_single_response_create(void);

#endif /** OCSP_SINGLE_RESPONSE_H_ @}*/
