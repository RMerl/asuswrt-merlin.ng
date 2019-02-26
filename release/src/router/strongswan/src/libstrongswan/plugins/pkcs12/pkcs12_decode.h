/*
 * Copyright (C) 2013 Tobias Brunner
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
 * @defgroup pkcs12_decode pkcs12_decode
 * @{ @ingroup pkcs12
 */

#ifndef PKCS12_DECODE_H_
#define PKCS12_DECODE_H_

#include <credentials/builder.h>
#include <credentials/containers/pkcs12.h>

/**
 * Load a PKCS#12 container.
 *
 * The argument list must contain a single BUILD_BLOB_ASN1_DER argument.
 *
 * @param type		type of the container, CONTAINER_PKCS12
 * @param args		builder_part_t argument list
 * @return			container, NULL on failure
 */
pkcs12_t *pkcs12_decode(container_type_t type, va_list args);

#endif /** PKCS12_DECODE_H_ @}*/
