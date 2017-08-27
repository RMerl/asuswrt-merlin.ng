/*
 * Copyright (C) 2011 Andreas Steffen
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
 * @defgroup tnccs_dynamic tnccs_dynamic
 * @ingroup cplugins
 *
 * @defgroup tnccs_dynamic_plugin tnccs_dynamic_plugin
 * @{ @ingroup tnccs_dynamic
 */

#ifndef TNCCS_DYNAMIC_PLUGIN_H_
#define TNCCS_DYNAMIC_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct tnccs_dynamic_plugin_t tnccs_dynamic_plugin_t;

/**
 * EAP-TNC plugin
 */
struct tnccs_dynamic_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** TNCCS_DYNAMIC_PLUGIN_H_ @}*/
