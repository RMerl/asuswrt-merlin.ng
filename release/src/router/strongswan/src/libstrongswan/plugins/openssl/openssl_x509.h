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
 * @defgroup openssl_x509 openssl_x509
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_X509_H_
#define OPENSSL_X509_H_

#include <credentials/certificates/x509.h>

typedef struct openssl_x509_t openssl_x509_t;

/**
 * X.509 certificate implementation using OpenSSL.
 */
struct openssl_x509_t {

	/**
	 * Implements x509_t interface.
	 */
	x509_t x509;
};

/**
 * Load a X.509 certificate.
 *
 * This function takes a BUILD_BLOB_ASN1_DER.
 *
 * @param type		certificate type, CERT_X509 only
 * @param args		builder_part_t argument list
 * @return			X.509 certificate, NULL on failure
 */
openssl_x509_t *openssl_x509_load(certificate_type_t type, va_list args);

#endif /** OPENSSL_X509_H_ @}*/
