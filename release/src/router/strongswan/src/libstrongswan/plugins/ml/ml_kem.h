/*
 * Copyright (C) 2024 Tobias Brunner
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
 * Quantum-safe key encapsulation implementation using ML-KEM.
 *
 * @defgroup ml_kem ml_kem
 * @{ @ingroup ml_p
 */

#ifndef ML_KEM_H_
#define ML_KEM_H_

#include <library.h>

/**
 * Creates a new key_exchange_t object.
 *
 * @param method		key exchange method
 * @return				key_exchange_t object, NULL if not supported
 */
key_exchange_t *ml_kem_create(key_exchange_method_t method);

#endif /** ML_KEM_H_ @}*/
