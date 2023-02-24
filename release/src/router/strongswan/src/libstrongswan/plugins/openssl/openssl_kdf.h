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
 * Implements key derivation functions (KDF) via OpenSSL, in particular prf+,
 * which is implemented via OpenSSL's HKDF implementation.
 *
 * @defgroup openssl_kdf openssl_kdf
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_KDF_H_
#define OPENSSL_KDF_H_

#include <crypto/kdfs/kdf.h>

/**
 * Creates a new kdf_t object.
 *
 * @param algo		algorithm to instantiate
 * @param args		algorithm-specific arguments
 * @return			kdf_t object, NULL if not supported
 */
kdf_t *openssl_kdf_create(key_derivation_function_t algo, va_list args);

#endif /** OPENSSL_KDF_H_ @}*/
