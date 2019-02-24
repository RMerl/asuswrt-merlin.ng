/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup sw_collector_dpkg_t sw_collector_dpkg
 * @{ @ingroup sw_collector
 */

#ifndef SW_COLLECTOR_DPKG_H_
#define SW_COLLECTOR_DPKG_H_

#include <library.h>

typedef struct sw_collector_dpkg_t sw_collector_dpkg_t;

/**
 * Software collector dpkg object
 */
struct sw_collector_dpkg_t {

	/**
	 * List of installed software identifiers managed by the
	 * Debian "dpkg" package manager
	 *
	 * @return				Enumerator
	 */
	enumerator_t* (*create_sw_enumerator)(sw_collector_dpkg_t *this);

	/**
	 * Destroy sw_collector_dpkg_t object
	 */
	void (*destroy)(sw_collector_dpkg_t *this);

};

/**
 * Create an sw_collector_dpkg_t instance
 */
sw_collector_dpkg_t* sw_collector_dpkg_create(void);

#endif /** SW_COLLECTOR_DPKG_H_ @}*/
