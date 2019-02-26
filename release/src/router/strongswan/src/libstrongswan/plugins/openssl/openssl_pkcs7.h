/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup openssl_pkcs7 openssl_pkcs7
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_PKCS7_H_
#define OPENSSL_PKCS7_H_

#include <credentials/containers/pkcs7.h>

/**
 * Load a generic PKCS#7 container.
 *
 * The argument list must contain a single BUILD_BLOB_ASN1_DER argument.
 *
 * @param type		type of the container, CONTAINER_PKCS7
 * @param args		builder_part_t argument list
 * @return			container, NULL on failure
 */
pkcs7_t *openssl_pkcs7_load(container_type_t type, va_list args);

#endif /** OPENSSL_PKCS7_H_ @}*/
