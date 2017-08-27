/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup load_tester_ipsec_i load_tester_ipsec
 * @{ @ingroup load_tester
 */

#ifndef LOAD_TESTER_IPSEC_H_
#define LOAD_TESTER_IPSEC_H_

#include <kernel/kernel_ipsec.h>

typedef struct load_tester_ipsec_t load_tester_ipsec_t;

/**
 * Implementation of a fake kernel ipsec interface for load testing.
 */
struct load_tester_ipsec_t {

	/**
	 * Implements kernel_ipsec_t interface
	 */
	kernel_ipsec_t interface;
};

/**
 * Create a faked kernel ipsec interface instance.
 *
 * @return			kernel_load_tester_ipsec_t instance
 */
load_tester_ipsec_t *load_tester_ipsec_create();

#endif /** LOAD_TESTER_IPSEC_H_ @}*/
