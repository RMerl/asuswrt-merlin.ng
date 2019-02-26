/*
 * Copyright (C) 2008-2009 Martin Willi
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
 * @defgroup x509_cert x509_cert
 * @{ @ingroup x509_p
 */

#ifndef X509_CERT_H_
#define X509_CERT_H_

typedef struct x509_cert_t x509_cert_t;

#include <credentials/builder.h>
#include <credentials/certificates/x509.h>

/**
 * Implementation of x509_t/certificate_t using own ASN1 parser.
 */
struct x509_cert_t {

	/**
	 * Implements the x509_t interface
	 */
	x509_t interface;
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
x509_cert_t *x509_cert_load(certificate_type_t type, va_list args);

/**
 * Generate a X.509 certificate.
 *
 * To issue a self-signed certificate, the function takes:
 * BUILD_SUBJECT, BUILD_SUBJECT_ALTNAMES, BUILD_SIGNING_KEY, BUILD_X509_FLAG,
 * BUILD_NOT_BEFORE_TIME, BUILD_NOT_AFTER_TIME, BUILD_SERIAL, BUILD_DIGEST_ALG.
 * To issue certificates from a CA, additionally pass:
 * BUILD_SIGNING_CERT and BUILD_PUBLIC_KEY.
 *
 * @param type		certificate type, CERT_X509 only
 * @param args		builder_part_t argument list
 * @return			X.509 certificate, NULL on failure
 */
x509_cert_t *x509_cert_gen(certificate_type_t type, va_list args);

#endif /** X509_CERT_H_ @}*/
