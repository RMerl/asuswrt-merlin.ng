/*
 * Copyright (C) 2015 Andreas Steffen
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
 * @defgroup sha3_p sha3
 * @ingroup plugins
 *
 * @defgroup sha3_plugin sha3_plugin
 * @{ @ingroup sha3_p
 */

#ifndef SHA3_PLUGIN_H_
#define SHA3_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct sha3_plugin_t sha3_plugin_t;

/**
 * Plugin implementing the SHA356, SHA384 and SHA512 algorithms in software.
 */
struct sha3_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** SHA3_PLUGIN_H_ @}*/
