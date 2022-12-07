/*
 * Copyright (C) 2014 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup bliss_p bliss
 * @ingroup plugins
 *
 * @defgroup bliss_plugin bliss_plugin
 * @{ @ingroup bliss_p
 */

#ifndef BLISS_PLUGIN_H_
#define BLISS_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct bliss_plugin_t bliss_plugin_t;

/**
 * Plugin implementing the BLISS post-quantum authentication algorithm
 */
struct bliss_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** BLISS_PLUGIN_H_ @}*/
