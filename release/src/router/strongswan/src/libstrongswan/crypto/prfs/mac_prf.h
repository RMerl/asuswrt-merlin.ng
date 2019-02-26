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
 * @defgroup mac_prf mac_prf
 * @{ @ingroup prf
 */

#ifndef MAC_PRF_H_
#define MAC_PRF_H_

#include <crypto/mac.h>
#include <crypto/prfs/prf.h>

/**
 * Creates an implementation of the prf_t interface using the provided mac_t
 * implementation.  Basically a simple wrapper to map the interface.
 *
 * @param mac		mac_t implementation
 * @return			prf_t object
 */
prf_t *mac_prf_create(mac_t *mac);

#endif /** MAC_PRF_H_ @}*/
