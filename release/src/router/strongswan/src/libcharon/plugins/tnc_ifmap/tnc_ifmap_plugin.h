/*
 * Copyright (C) 2011-2013 Andreas Steffen
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
 * @defgroup tnc_ifmap tnc_ifmap
 * @ingroup cplugins
 *
 * @defgroup tnc_ifmap_plugin tnc_ifmap_plugin
 * @{ @ingroup tnc_ifmap
 */

#ifndef TNC_IFMAP_PLUGIN_H_
#define TNC_IFMAP_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct tnc_ifmap_plugin_t tnc_ifmap_plugin_t;

/**
 * TNC IF-MAP plugin
 */
struct tnc_ifmap_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** TNC_IFMAP_PLUGIN_H_ @}*/
