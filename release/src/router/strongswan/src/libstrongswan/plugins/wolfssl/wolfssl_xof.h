/*
 * Copyright (C) 2021 Andreas Steffen, strongSec GmbH
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
 * Implementation of the SHAKE128/256 XOF algorithm using OpenSSL.
 *
 * @defgroup wolfssl_xof wolfssl_xof
 * @{ @ingroup wolfssl_p
 */

#ifndef WOLFSSL_XOF_H_
#define WOLFSSL_XOF_H_

#include <library.h>

/**
 * Creates a new xof_t object.
 *
 * @param algorithm		XOF algorithm to create
 * @return				object, NULL if not supported
 */
xof_t *wolfssl_xof_create(ext_out_function_t algorithm);

#endif /** WOLFSSL_XOF_H_ @}*/
