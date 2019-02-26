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
 * @defgroup tnc_tnccs tnc_tnccs
 * @ingroup tplugins
 *
 * @defgroup tnc_tnccs_plugin tnc_tnccs_plugin
 * @{ @ingroup tnc_tnccs
 */

#ifndef TNC_TNCCS_PLUGIN_H_
#define TNC_TNCCS_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct tnc_tnccs_plugin_t tnc_tnccs_plugin_t;

/**
 * TNCCS manager plugin
 */
struct tnc_tnccs_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** TNC_TNCCS_PLUGIN_H_ @}*/
