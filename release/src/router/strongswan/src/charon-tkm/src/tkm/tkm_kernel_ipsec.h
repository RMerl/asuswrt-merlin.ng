/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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
 * @defgroup tkm-kernel-ipsec kernel ipsec
 * @{ @ingroup tkm
 */

#ifndef TKM_KERNEL_IPSEC_H_
#define TKM_KERNEL_IPSEC_H_

#include <kernel/kernel_ipsec.h>

typedef struct tkm_kernel_ipsec_t tkm_kernel_ipsec_t;

/**
 * TKM implementation of the kernel ipsec interface.
 */
struct tkm_kernel_ipsec_t {

	/**
	 * Implements kernel_ipsec_t interface
	 */
	kernel_ipsec_t interface;
};

/**
 * Create a TKM kernel ipsec interface instance.
 *
 * @return			tkm_kernel_ipsec_t instance
 */
tkm_kernel_ipsec_t *tkm_kernel_ipsec_create();

#endif /** TKM_KERNEL_IPSEC_H_ @}*/
