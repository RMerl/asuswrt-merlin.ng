/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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
 * @defgroup curve25519_drv_portable curve25519_drv_portable
 * @{ @ingroup curve25519_p
 */

#include "curve25519_drv.h"

#ifndef CURVE25519_DRV_PORTABLE_H_
#define CURVE25519_DRV_PORTABLE_H_

/**
 * Create a curve25519_drv_portable instance.
 */
curve25519_drv_t *curve25519_drv_portable_create();

#endif /** CURVE25519_DRV_PORTABLE_H_ @}*/
