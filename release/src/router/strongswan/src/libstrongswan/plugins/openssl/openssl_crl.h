/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup openssl_crl openssl_crl
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_CRL_H_
#define OPENSSL_CRL_H_

typedef struct openssl_crl_t openssl_crl_t;

#include <credentials/certificates/crl.h>

/**
 * X.509 Certificate Revocation list implemented with OpenSSL.
 */
struct openssl_crl_t {

	/**
	 * Implements the crl_t interface.
	 */
	crl_t crl;
};

/**
 * Load a X.509 CRL using OpenSSL.
 *
 * @param type		certificate type, CERT_X509_CRL only
 * @param args		builder_part_t argument list
 * @return			X.509 CRL, NULL on failure
 */
openssl_crl_t *openssl_crl_load(certificate_type_t type, va_list args);

#endif /** OPENSSL_CRL_H_ @}*/
