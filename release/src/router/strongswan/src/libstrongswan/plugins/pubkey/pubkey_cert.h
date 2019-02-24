/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup pubkey_cert pubkey_cert
 * @{ @ingroup certificates
 */

#ifndef PUBKEY_CERT_H_
#define PUBKEY_CERT_H_

#include <credentials/builder.h>
#include <credentials/certificates/certificate.h>

typedef struct pubkey_cert_t pubkey_cert_t;

/**
 * A trusted public key wrapped into certificate of type CERT_TRUSTED_PUBKEY.
 */
struct pubkey_cert_t {

	/**
	 * Implements certificate_t.
	 */
	certificate_t interface;

	/**
	 * Set the subject of the trusted public key.
	 *
	 * @param subject	subject to be set
	 */
	void (*set_subject)(pubkey_cert_t *this, identification_t *subject);
};

/**
 * Create a trusted public key cert using a public key.
 *
 * The build accepts a BUILD_PUBLIC_KEY or a BUILD_BLOB_ASN1_DER part.
 *
 * @param type		type of the certificate, must be CERT_pubkey_cert
 * @param args		builder_part_t argument list
 * @return 			pubkey_cert_t, NULL on failure
 */
pubkey_cert_t *pubkey_cert_wrap(certificate_type_t type, va_list args);

#endif /** PUBKEY_CERT_H_ @}*/
