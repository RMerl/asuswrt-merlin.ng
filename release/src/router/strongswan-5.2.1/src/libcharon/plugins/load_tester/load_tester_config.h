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
 * @defgroup load_tester_config_t load_tester_config
 * @{ @ingroup load_tester
 */

#ifndef LOAD_TESTER_CONFIG_H_
#define LOAD_TESTER_CONFIG_H_

#include <config/backend.h>

typedef struct load_tester_config_t load_tester_config_t;

/**
 * Provide configurations for load testing.
 */
struct load_tester_config_t {

	/**
	 * Implements backend_t interface
	 */
	backend_t backend;

	/**
	 * Delete external IP if it was dynamically installed.
	 *
	 * @param ip			external IP
	 */
	void (*delete_ip)(load_tester_config_t *this, host_t *ip);

	/**
	 * Destroy the backend.
	 */
	void (*destroy)(load_tester_config_t *this);
};

/**
 * Create a configuration backend for load testing.
 *
 * @return			configuration backend
 */
load_tester_config_t *load_tester_config_create();

#endif /** LOAD_TESTER_CONFIG_H_ @}*/
