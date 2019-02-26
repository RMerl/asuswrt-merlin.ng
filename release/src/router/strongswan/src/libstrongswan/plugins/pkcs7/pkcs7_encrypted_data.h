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
 * @defgroup pkcs7_encrypted_data pkcs7_encrypted_data
 * @{ @ingroup pkcs7p
 */

#ifndef PKCS7_ENCRYPTED_DATA_H_
#define PKCS7_ENCRYPTED_DATA_H_

#include <credentials/builder.h>
#include <credentials/containers/pkcs7.h>

/**
 * Parse a PKCS#7 encrypted-data container.
 *
 * @param encoding	full contentInfo encoding
 * @param content	DER encoded content from contentInfo
 * @return			CONTAINER_PKCS7_ENCRYPTED_DATA container, NULL on failure
 */
pkcs7_t *pkcs7_encrypted_data_load(chunk_t encoding, chunk_t content);

#endif /** PKCS7_ENCRYPTED_DATA_H_ @}*/
