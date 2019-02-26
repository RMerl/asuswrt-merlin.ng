/*
 * Copyright (C) 2018 Tobias Brunner
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
 * kernel_net_t implementation used for exchange unit tests.  Simply returns
 * an IP address so it seems we're connected.
 *
 * @defgroup mock_net mock_net
 * @{ @ingroup test_utils_c
 */

#ifndef MOCK_NET_H_
#define MOCK_NET_H_

#include <kernel/kernel_net.h>

/**
 * Create an instance of kernel_net_t
 *
 * @return		created object
 */
kernel_net_t *mock_net_create();

#endif /** MOCK_NET_H_ @}*/
