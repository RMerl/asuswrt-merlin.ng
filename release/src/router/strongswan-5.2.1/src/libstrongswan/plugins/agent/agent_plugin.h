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
 * @defgroup agent_p agent
 * @ingroup plugins
 *
 * @defgroup agent_plugin agent_plugin
 * @{ @ingroup agent_p
 */

#ifndef AGENT_PLUGIN_H_
#define AGENT_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct agent_plugin_t agent_plugin_t;

/**
 * Plugin to use private keys loaded in a ssh-agent.
 */
struct agent_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** AGENT_PLUGIN_H_ @}*/
