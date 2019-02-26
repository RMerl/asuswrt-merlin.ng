/*
 * Copyright (C) 2016 Andreas Steffen
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
 * @defgroup mgf1_p mgf1
 * @ingroup plugins
 *
 * @defgroup mgf1_plugin mgf1_plugin
 * @{ @ingroup mgf1_p
 */

#ifndef MGF1_PLUGIN_H_
#define MGF1_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct mgf1_plugin_t mgf1_plugin_t;

/**
 * Plugin implementing the MGF1 Mask Generator Function in software.
 */
struct mgf1_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** MGF1_PLUGIN_H_ @}*/
