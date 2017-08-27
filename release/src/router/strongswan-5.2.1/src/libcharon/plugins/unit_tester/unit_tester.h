/*
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup unit_tester unit_tester
 * @{ @ingroup cplugins
 */

#ifndef UNIT_TESTER_H_
#define UNIT_TESTER_H_

#include <plugins/plugin.h>

typedef struct unit_tester_t unit_tester_t;

/**
 * Unit testing plugin.
 *
 * The unit testing plugin runs tests on plugin initialization. Tests are
 * defined in tests.h using the DEFINE_TEST macro. Implementation of the
 * tests is done in the tests folder. Each test has uses a function which
 * returns TRUE for success or FALSE for failure.
 */
struct unit_tester_t {

	/**
	 * Implements the plugin interface.
	 */
	plugin_t plugin;
};

#endif /** UNIT_TESTER_H_ @}*/
