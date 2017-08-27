/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup ocsp_response_wrapper ocsp_response_wrapper
 * @{ @ingroup sets
 */

#ifndef OCSP_RESPONSE_WRAPPER_H_
#define OCSP_RESPONSE_WRAPPER_H_

#include <credentials/credential_set.h>
#include <credentials/certificates/ocsp_response.h>

typedef struct ocsp_response_wrapper_t ocsp_response_wrapper_t;

/**
 * A wrapper around ocsp_response_t to handle it like a credential set.
 */
struct ocsp_response_wrapper_t {

	/**
	 * implements credential_set_t
	 */
	credential_set_t set;

	/**
	 * Destroy a ocsp_response_wrapper instance.
	 */
	void (*destroy)(ocsp_response_wrapper_t *this);
};

/**
 * Create a ocsp_response_wrapper instance.
 *
 * @param response	the wrapped OCSP response
 * @return			wrapper around response
 */
ocsp_response_wrapper_t *ocsp_response_wrapper_create(ocsp_response_t *response);

#endif /** OCSP_RESPONSE_WRAPPER_H_ @}*/
