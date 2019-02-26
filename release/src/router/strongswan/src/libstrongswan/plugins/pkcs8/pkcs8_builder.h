/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup pkcs8_builder pkcs8_builder
 * @{ @ingroup pkcs8
 */

#ifndef PKCS8_BUILDER_H_
#define PKCS8_BUILDER_H_

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

/**
 * Load an RSA or ECDSA private key from PKCS#8 data.
 *
 * @param type		type of the key, KEY_RSA or KEY_ECDSA
 * @param args		builder_part_t argument list
 * @return			private key, NULL on failure
 */
private_key_t *pkcs8_private_key_load(key_type_t type, va_list args);

#endif /** PKCS8_BUILDER_H_ @}*/
