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
 * @defgroup tnccs_11 tnccs_11
 * @ingroup tplugins
 *
 * @defgroup tnccs_11_plugin tnccs_11_plugin
 * @{ @ingroup tnccs_11
 */

#ifndef TNCCS_11_PLUGIN_H_
#define TNCCS_11_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct tnccs_11_plugin_t tnccs_11_plugin_t;

/**
 * EAP-TNC plugin
 */
struct tnccs_11_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** TNCCS_11_PLUGIN_H_ @}*/
