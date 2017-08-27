/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup eap_sim_file eap_sim_file
 * @ingroup cplugins
 *
 * @defgroup eap_sim_file_plugin eap_sim_file_plugin
 * @{ @ingroup eap_sim_file
 */

#ifndef EAP_SIM_FILE_PLUGIN_H_
#define EAP_SIM_FILE_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_sim_file_plugin_t eap_sim_file_plugin_t;

/**
 * Plugin to provide a SIM card/provider on top of a triplet file.
 */
struct eap_sim_file_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** EAP_SIM_FILE_PLUGIN_H_ @}*/
