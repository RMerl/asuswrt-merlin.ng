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
 * @defgroup tnc_pdp tnc_pdp
 * @ingroup cplugins
 *
 * @defgroup tnc_pdp_plugin tnc_pdp_plugin
 * @{ @ingroup tnc_pdp
 */

#ifndef TNC_PDP_PLUGIN_H_
#define TNC_PDP_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct tnc_pdp_plugin_t tnc_pdp_plugin_t;

/**
 * TNC-PDP plugin
 */
struct tnc_pdp_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** TNC_PDP_PLUGIN_H_ @}*/
