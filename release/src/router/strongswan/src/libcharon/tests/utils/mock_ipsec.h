/*
 * Copyright (C) 2016-2017 Tobias Brunner
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
 * kernel_ipsec_t implementation used for exchange unit tests.  Currently
 * returns sequential SPIs, and keeps track of installed SAs.
 *
 * @defgroup mock_ipsec mock_ipsec
 * @{ @ingroup test_utils_c
 */

#ifndef MOCK_IPSEC_H_
#define MOCK_IPSEC_H_

#include <kernel/kernel_ipsec.h>

/**
 * Create an instance of kernel_ipsec_t
 *
 * @return		created object
 */
kernel_ipsec_t *mock_ipsec_create();

/**
 * Enumerate the installed SAs
 *
 * @return		enumerator over (ike_sa_t*, uint32_t)
 */
enumerator_t *mock_ipsec_create_sa_enumerator();

#endif /** MOCK_IPSEC_H_ @}*/
