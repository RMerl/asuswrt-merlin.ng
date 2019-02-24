/*
 * Copyright (C) 2010 Andreas Steffen
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
 * @defgroup tnccs_20 tnccs_20
 * @ingroup tplugins
 *
 * @defgroup tnccs_20_plugin tnccs_20_plugin
 * @{ @ingroup tnccs_20
 */

#ifndef TNCCS_20_PLUGIN_H_
#define TNCCS_20_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct tnccs_20_plugin_t tnccs_20_plugin_t;

/**
 * EAP-TNC plugin
 */
struct tnccs_20_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** TNCCS_20_PLUGIN_H_ @}*/
