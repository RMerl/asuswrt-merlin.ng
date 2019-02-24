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
 * @defgroup gmp_p gmp
 * @ingroup plugins
 *
 * @defgroup gmp_plugin gmp_plugin
 * @{ @ingroup gmp_p
 */

#ifndef GMP_PLUGIN_H_
#define GMP_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct gmp_plugin_t gmp_plugin_t;

/**
 * Plugin implementing asymmetric crypto algorithms using the GNU MP library.
 */
struct gmp_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** GMP_PLUGIN_H_ @}*/
