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
 * @defgroup tnc_imv tnc_imv
 * @ingroup tplugins
 *
 * @defgroup tnc_imv_plugin tnc_imv_plugin
 * @{ @ingroup tnc_imv
 */

#ifndef TNC_IMV_PLUGIN_H_
#define TNC_IMV_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct tnc_imv_plugin_t tnc_imv_plugin_t;

/**
 * TNC IMV plugin
 */
struct tnc_imv_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** TNC_IMV_PLUGIN_H_ @}*/
