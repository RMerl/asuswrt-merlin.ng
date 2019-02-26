/*
 * Copyright (C) 2013 Tobias Brunner
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
 * @defgroup pem_builder pem_builder
 * @{ @ingroup pem_p
 */

#ifndef PEM_BUILDER_H_
#define PEM_BUILDER_H_

#include <credentials/builder.h>
#include <credentials/credential_factory.h>
#include <credentials/keys/private_key.h>
#include <credentials/certificates/certificate.h>
#include <credentials/containers/container.h>

/**
 * Load PEM encoded private keys.
 *
 * @param type		type of the key
 * @param args		builder_part_t argument list
 * @return 			private key, NULL if failed
 */
private_key_t *pem_private_key_load(key_type_t type, va_list args);

/**
 * Load PEM encoded public keys.
 *
 * @param type		type of the key
 * @param args		builder_part_t argument list
 * @return 			public key, NULL if failed
 */
public_key_t *pem_public_key_load(key_type_t type, va_list args);

/**
 * Build PEM encoded certificates.
 *
 * @param type		type of the certificate
 * @param args		builder_part_t argument list
 * @return 			certificate, NULL if failed
 */
certificate_t *pem_certificate_load(certificate_type_t type, va_list args);

/**
 * Build PEM encoded containers.
 *
 * @param type		type of the container
 * @param args		builder_part_t argument list
 * @return 			container, NULL if failed
 */
container_t *pem_container_load(container_type_t type, va_list args);

#endif /** PEM_BUILDER_H_ @}*/

