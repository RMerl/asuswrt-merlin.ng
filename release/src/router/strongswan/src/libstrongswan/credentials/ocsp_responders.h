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

/**
 * @defgroup ocsp_responders ocsp_responders
 * @{ @ingroup credentials
 */

#ifndef OCSP_RESPONDERS_H_
#define OCSP_RESPONDERS_H_

typedef struct ocsp_responders_t ocsp_responders_t;

#include <credentials/ocsp_responder.h>

/**
 * Manages OCSP responders.
 */
struct ocsp_responders_t {

	/**
	 * Check the status of a certificate given by its serial number
	 *
	 * @param cacert			X.509 certificate of issuer CA
	 * @param serial_number		serial number of the certificate to be checked
	 * @param revocation_time	receives time of revocation, if revoked
	 * @param reason	        receives reason of revocation, if revoked
	 * @return					certificate validation status
	 */
	cert_validation_t (*get_status)(ocsp_responders_t *this,
									certificate_t *cacert,
									chunk_t serial_number,
									time_t *revocation_time,
									crl_reason_t *revocation_reason);

	/**
	 * Register an OCSP responder with this manager.
	 *
	 * @param responder			OCSP responder to register
	 */
	void (*add_responder)(ocsp_responders_t *this,
						  ocsp_responder_t *responder);

	/**
	 * Unregister an OCSP responder from this manager.
	 *
	 * @param responder			OCSP responder to unregister
	 */
	void (*remove_responder)(ocsp_responders_t *this,
							 ocsp_responder_t *responder);

	/**
	 * Destroy a ocsp_responders_t instance.
	 */
	void (*destroy)(ocsp_responders_t *this);
};

/**
 * Create a ocsp_responders_t instance.
 */
ocsp_responders_t *ocsp_responders_create();

#endif /** OCSP_RESPONDERS_H_ @}*/
