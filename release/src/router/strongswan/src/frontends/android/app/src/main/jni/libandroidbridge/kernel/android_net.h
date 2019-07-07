/*
 * Copyright (C) 2012-2015 Tobias Brunner
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
 * @defgroup android_net android_net
 * @{ @ingroup android_kernel
 */

#ifndef ANDROID_NET_H_
#define ANDROID_NET_H_

#include <library.h>
#include <kernel/kernel_net.h>

/**
 * Create an Android-specific kernel_net_t instance.
 *
 * @return			kernel_net_t instance
 */
kernel_net_t *kernel_android_net_create();


#endif /** ANDROID_NET_H_ @}*/
