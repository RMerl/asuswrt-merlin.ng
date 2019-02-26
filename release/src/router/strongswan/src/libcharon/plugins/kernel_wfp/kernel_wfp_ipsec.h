/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup kernel_wfp_ipsec kernel_wfp_ipsec
 * @{ @ingroup kernel_wfp
 */

#ifndef KERNEL_WFP_IPSEC_H_
#define KERNEL_WFP_IPSEC_H_

#include <library.h>
#include <kernel/kernel_ipsec.h>

typedef struct kernel_wfp_ipsec_t kernel_wfp_ipsec_t;

/**
 * Windows Filter Platform based IPsec kernel backend.
 */
struct kernel_wfp_ipsec_t {

	/**
	 * Implements kernel_ipsec_t interface
	 */
	kernel_ipsec_t interface;
};

/**
 * Create WFP kernel interface instance.
 *
 * @return			kernel_wfp_ipsec_t instance
 */
kernel_wfp_ipsec_t *kernel_wfp_ipsec_create();

#endif /** KERNEL_WFP_IPSEC_H_ @}*/
