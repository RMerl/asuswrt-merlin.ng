/*
 * Copyright (C) 2002 Ueli Galizzi, Ariane Seiler
 * Copyright (C) 2003 Martin Berner, Lukas Suter
 * Copyright (C) 2002-2008 Andreas Steffen
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup x509_ac x509_ac
 * @{ @ingroup x509_p
 */

#ifndef X509_AC_H_
#define X509_AC_H_

#include <credentials/builder.h>
#include <credentials/certificates/ac.h>

typedef struct x509_ac_t x509_ac_t;

/**
 * Implementation of ac_t using own ASN1 parser.
 */
struct x509_ac_t {

	/**
	 * Implements the ac_t interface
	 */
	ac_t interface;
};

/**
 * Load a X.509 attribute certificate.
 *
 * @param type		certificate type, CERT_X509_AC only
 * @param args		builder_part_t argument list
 * @return			X.509 Attribute certificate, NULL on failure
 */
x509_ac_t *x509_ac_load(certificate_type_t type, va_list args);

/**
 * Generate a X.509 attribute certificate.
 *
 * Accepted build parts:
 *  BUILD_USER_CERT:	user certificate
 *  BUILD_SIGNER_CERT:	signer certificate
 *  BUILD_SIGNER_KEY:	signer private key
 *  BUILD_SERIAL:		serial number
 *  BUILD_GROUP_ATTR:	group attribute, several possible
 *
 * @param type		certificate type, CERT_X509_AC only
 * @param args		builder_part_t argument list
 * @return			X.509 Attribute certificate, NULL on failure
 */
x509_ac_t *x509_ac_gen(certificate_type_t type, va_list args);

#endif /** X509_AC_H_ @}*/
