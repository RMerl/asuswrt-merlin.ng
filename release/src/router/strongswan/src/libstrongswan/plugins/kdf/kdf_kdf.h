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
 * Implements a KDF wrapper around PRFs, and prf+ as defined in RFC 7296,
 * section 2.13:
 *
 * @verbatim
     prf+ (K,S) = T1 | T2 | T3 | T4 | ...

     where:
     T1 = prf (K, S | 0x01)
     T2 = prf (K, T1 | S | 0x02)
     T3 = prf (K, T2 | S | 0x03)
     T4 = prf (K, T3 | S | 0x04)
     ...
   @endverbatim
 *
 * @defgroup kdf_kdf kdf_kdf
 * @{ @ingroup kdf_p
 */

#ifndef KDF_KDF_H_
#define KDF_KDF_H_

#include <crypto/kdfs/kdf.h>

/**
 * Create a kdf_t object
 *
 * @param algo			algorithm to instantiate
 * @param args			pseudo_random_function_t of the underlying PRF
 * @return				kdf_t object, NULL if not supported
 */
kdf_t *kdf_kdf_create(key_derivation_function_t algo, va_list args);

#endif /** KDF_KDF_H_ @}*/
