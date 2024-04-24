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
 * @defgroup ocsp_responder ocsp_responder
 * @{ @ingroup credentials
 */

#ifndef OCSP_RESPONDER_H_
#define OCSP_RESPONDER_H_

typedef struct ocsp_responder_t ocsp_responder_t;

#include <credentials/certificates/crl.h>

/**
 * OCSP responder object.
 */
struct ocsp_responder_t {

	/**
	 * Check the status of a certificate given by its serial number
	 *
	 * @note Return VALIDATION_SKIPPED if not responsible for the given CA
	 *
	 * @param cacert			X.509 certificate of issuer CA
	 * @param serial_number		serial number of the certificate to be checked
	 * @param revocation_time	receives time of revocation, if revoked
	 * @param reason	        receives reason of revocation, if revoked
	 * @return					certificate validation status
	 */
	cert_validation_t (*get_status)(ocsp_responder_t *this,
									certificate_t *cacert,
									chunk_t serial_number,
									time_t *revocation_time,
									crl_reason_t *revocation_reason);

	/**
	 * Destroy an ocsp_responder_t object.
	 */
	void (*destroy)(ocsp_responder_t *this);

};

#endif /** OCSP_RESPONDER_H_ @}*/
