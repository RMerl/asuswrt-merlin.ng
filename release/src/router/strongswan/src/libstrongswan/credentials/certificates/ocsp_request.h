/*
 * Copyright (C) 2019 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup ocsp_request ocsp_request
 * @{ @ingroup certificates
 */

#ifndef OCSP_REQUEST_H_
#define OCSP_REQUEST_H_

#include <credentials/certificates/certificate.h>

typedef struct ocsp_request_t ocsp_request_t;

/**
 * OCSP request message.
 */
struct ocsp_request_t {

	/**
	 * Implements certificate_t interface
	 */
	certificate_t interface;

	/**
	 * Get the nonce sent in this OCSP request.
	 *
	 * @return					nonce in the request (internal data)
	 */
	chunk_t (*get_nonce)(ocsp_request_t *this);
};

#endif /** OCSP_REQUEST_H_ @}*/
