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
 * @defgroup pkcs1_builder pkcs1_builder
 * @{ @ingroup pkcs1
 */

#ifndef PKCS1_BUILDER_H_
#define PKCS1_BUILDER_H_

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

/**
 * Load a generic or an RSA public key from PKCS#1 data.
 *
 * @param type		type of the key, either KEY_ANY or KEY_RSA
 * @param args		builder_part_t argument list
 * @return 			public key, NULL on failure
 */
public_key_t *pkcs1_public_key_load(key_type_t type, va_list args);

/**
 * Load a RSA public key from PKCS#1 data.
 *
 * @param type		type of the key, KEY_RSA
 * @param args		builder_part_t argument list
 * @return 			private key, NULL on failure
 */
private_key_t *pkcs1_private_key_load(key_type_t type, va_list args);

#endif /** PKCS1_BUILDER_H_ @}*/
