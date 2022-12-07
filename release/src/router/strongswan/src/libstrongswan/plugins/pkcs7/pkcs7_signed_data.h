/*
 * Copyright (C) 2012 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup pkcs7_signed_data pkcs7_signed_data
 * @{ @ingroup pkcs7p
 */

#ifndef PKCS7_SIGNED_DATA_H_
#define PKCS7_SIGNED_DATA_H_

#include <credentials/builder.h>
#include <credentials/containers/pkcs7.h>

/**
 * Parse a PKCS#7 signed-data container.
 *
 * @param encoding	full contentInfo encoding
 * @param content	DER encoded content from contentInfo
 * @return			CONTAINER_PKCS7_SIGNED_DATA container, NULL on failure
 */
pkcs7_t *pkcs7_signed_data_load(chunk_t encoding, chunk_t content);

/**
 * Generate a PKCS#7 signed-data container.
 *
 * @param type		container type, must be CONTAINER_PKCS7_SIGNED_DATA
 * @param args		builder_t arguments to use.
 */
pkcs7_t *pkcs7_signed_data_gen(container_type_t type, va_list args);

#endif /** PKCS7_SIGNED_DATA_H_ @}*/
