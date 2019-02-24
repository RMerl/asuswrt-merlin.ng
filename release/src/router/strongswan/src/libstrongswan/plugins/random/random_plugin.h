/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup random_p random
 * @ingroup plugins
 *
 * @defgroup random_plugin random_plugin
 * @{ @ingroup random_p
 */

#ifndef RANDOM_PLUGIN_H_
#define RANDOM_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct random_plugin_t random_plugin_t;

/**
 * Plugin implementing a RNG reading from /dev/[u]random.
 */
struct random_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

/**
 * Get the /dev/random file descriptor
 */
int random_plugin_get_dev_random();

/**
 * Get the /dev/urandom file descriptor
 */
int random_plugin_get_dev_urandom();

/**
 * Must strong randomness be equivalent to true randomness?
 */
bool random_plugin_get_strong_equals_true();

#endif /** RANDOM_PLUGIN_H_ @}*/
