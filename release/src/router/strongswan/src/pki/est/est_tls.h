/*
 * Copyright (C) 2022 Andreas Steffen
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
 * @defgroup est_tls est_tls
 * @{ @ingroup pki
 */

#ifndef EST_TLS_H_
#define EST_TLS_H_

#include <library.h>
#include <credentials/certificates/certificate.h>

#define EST_HTTP_CODE_OK         200
#define EST_HTTP_CODE_ACCEPTED   202

typedef struct est_tls_t est_tls_t;

/**
 * EST (RFC 7030) Operations
 */
typedef enum {
    EST_CACERTS,
    EST_SIMPLE_ENROLL,
    EST_SIMPLE_REENROLL,
    EST_FULL_CMC,
    EST_SERVER_KEYGEN,
    EST_CSR_ATTRS
} est_op_t;

/**
 * TLS Interface for sending and receiving HTTPS messages
 */
struct est_tls_t {

	/**
	 * Send a https request and get a response back
	 *
	 * @param option		EST operation
	 * @param in			HTTP POST input data
	 * @param out			HTTP response
	 * @param http_code		HTTP status code
     * @param retry_after   Retry time in seconds
	 * @result				TRUE if successful
	 */
	bool (*request)(est_tls_t *this, est_op_t op, chunk_t in, chunk_t *out,
					u_int *http_code, u_int *retry_after);

	/**
	 * Destroy an est_tls_t object.
	 */
	void (*destroy)(est_tls_t *this);
};

/**
 * Create a est_tls instance.
 *
 * @param uri			URI (https://...)
 * @param label         Optional EST server label
 * @param client_cert   Optional client certificate
 * @param user_pass		Optional username:password for HTTP Basic Authentication
 */
est_tls_t *est_tls_create(char *uri, char *label, certificate_t *client_cert,
						  char *user_pass);

#endif /** EST_TLS_H_ @}*/
