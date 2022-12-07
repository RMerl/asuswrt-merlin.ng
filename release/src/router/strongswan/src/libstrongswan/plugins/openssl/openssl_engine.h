/*
 * Copyright (C) 2022 Tobias Brunner
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
 * Compatibility code for legacy ENGINE support.
 *
 * @defgroup openssl_engine openssl_engine
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_ENGINE_H_
#define OPENSSL_ENGINE_H_

#include <credentials/keys/private_key.h>

/**
 * Load a private key from a token/ENGINE.
 *
 * @param type		key type to load
 * @param args		build arguments
 */
private_key_t *openssl_private_key_connect(key_type_t type, va_list args);

/**
 * Initialize ENGINE support.
 */
void openssl_engine_init();

/**
 * Deinitialize ENGINE support.
 */
void openssl_engine_deinit();

#endif /** OPENSSL_ENGINE_H_ @}*/
