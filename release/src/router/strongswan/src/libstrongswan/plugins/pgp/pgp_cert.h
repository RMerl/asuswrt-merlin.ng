/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup pgp_cert pgp_cert
 * @{ @ingroup pgp
 */

#ifndef PGP_CERT_H_
#define PGP_CERT_H_

#include <credentials/certificates/pgp_certificate.h>

typedef struct pgp_cert_t pgp_cert_t;

/**
 * PGP certificate implementation.
 */
struct pgp_cert_t {

	/**
	 * Implements pgp_certificate_t.
	 */
	pgp_certificate_t interface;
};

/**
 * Load a PGP certificate.
 *
 * @param type		type of the certificate, CERT_GPG
 * @param args		builder_part_t argument list
 * @return 			builder instance
 */
pgp_cert_t *pgp_cert_load(certificate_type_t type, va_list args);

#endif /** PGP_CERT_H_ @}*/
