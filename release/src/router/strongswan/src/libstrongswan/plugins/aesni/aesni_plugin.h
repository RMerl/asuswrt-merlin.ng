/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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
 * @defgroup aesni aesni
 * @ingroup plugins
 *
 * @defgroup aesni_plugin aesni_plugin
 * @{ @ingroup aesni
 */

#ifndef AESNI_PLUGIN_H_
#define AESNI_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct aesni_plugin_t aesni_plugin_t;

/**
 * Plugin providing crypto primitives based on Intel AES-NI instructions.
 */
struct aesni_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** AESNI_PLUGIN_H_ @}*/
