/*
 * Copyright (C) 2008-2009 Martin Willi
 * Copyright (C) 2009 Andreas Steffen
 *
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
 * @defgroup x509_pkcs10 x509_pkcs10
 * @{ @ingroup x509_p
 */

#ifndef X509_PKCS10_H_
#define X509_PKCS10_H_

typedef struct x509_pkcs10_t x509_pkcs10_t;

#include <credentials/builder.h>
#include <credentials/certificates/pkcs10.h>

/**
 * Implementation of pkcs10_t/certificate_t using own ASN.1 parser.
 */
struct x509_pkcs10_t {

	/**
	 * Implements the pkcs10_t interface
	 */
	pkcs10_t interface;
};

/**
 * Load a PKCS#10 certificate.
 *
 * This function takes a BUILD_BLOB_ASN1_DER.
 *
 * @param type		certificate type, CERT_PKCS10_REQUEST only
 * @param args		builder_part_t argument list
 * @return			PKCS#10 certificate request, NULL on failure
 */
x509_pkcs10_t *x509_pkcs10_load(certificate_type_t type, va_list args);

/**
 * Generate a PKCS#10 certificate request.
 *
 * To issue a self-signed certificate request, the function takes:
 * BUILD_SUBJECT, BUILD_SUBJECT_ALTNAMES, BUILD_SIGNING_KEY, BUILD_DIGEST_ALG.
 *
 * @param type		certificate type, CERT_PKCS10_REQUEST only
 * @param args		builder_part_t argument list
 * @return			PKCS#10 certificate request, NULL on failure
 */
x509_pkcs10_t *x509_pkcs10_gen(certificate_type_t type, va_list args);

#endif /** X509_PKCS10_H_ @}*/
