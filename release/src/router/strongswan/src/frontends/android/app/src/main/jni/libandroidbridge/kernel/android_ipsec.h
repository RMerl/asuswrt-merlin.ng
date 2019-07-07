/*
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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
 * @defgroup kernel_android_ipsec kernel_android_ipsec
 * @{ @ingroup android_kernel
 */

#ifndef KERNEL_ANDROID_IPSEC_H_
#define KERNEL_ANDROID_IPSEC_H_

#include <library.h>
#include <kernel/kernel_ipsec.h>

typedef struct kernel_android_ipsec_t kernel_android_ipsec_t;

/**
 * Implementation of the ipsec interface using libipsec on Android
 */
struct kernel_android_ipsec_t {

	/**
	 * Implements kernel_ipsec_t interface
	 */
	kernel_ipsec_t interface;
};

/**
 * Create a android ipsec interface instance.
 *
 * @return			kernel_android_ipsec_t instance
 */
kernel_android_ipsec_t *kernel_android_ipsec_create();

#endif /** KERNEL_ANDROID_IPSEC_H_ @}*/
