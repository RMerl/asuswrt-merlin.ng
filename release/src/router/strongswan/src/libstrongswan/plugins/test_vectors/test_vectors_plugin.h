/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup test_vectors_p test_vectors
 * @ingroup plugins
 *
 * @defgroup test_vectors_plugin test_vectors_plugin
 * @{ @ingroup test_vectors_p
 */

#ifndef TEST_VECTORS_PLUGIN_H_
#define TEST_VECTORS_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct test_vectors_plugin_t test_vectors_plugin_t;

/**
 * Plugin providing various crypto test vectors.
 */
struct test_vectors_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** TEST_VECTORS_PLUGIN_H_ @}*/
