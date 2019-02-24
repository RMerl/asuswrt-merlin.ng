/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup af_alg af_alg
 * @ingroup plugins
 *
 * @defgroup af_alg_plugin af_alg_plugin
 * @{ @ingroup af_alg
 */

#ifndef AF_ALG_PLUGIN_H_
#define AF_ALG_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct af_alg_plugin_t af_alg_plugin_t;

/**
 * Plugin providing the AF_ALG interface to the Linux Crypto API.
 */
struct af_alg_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** AF_ALG_PLUGIN_H_ @}*/
