/*
 * Copyright (C) 2016 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup bypass_lan bypass_lan
 * @ingroup cplugins
 *
 * @defgroup bypass_lan_plugin bypass_lan_plugin
 * @{ @ingroup bypass_lan
 */

#ifndef BYPASS_LAN_PLUGIN_H_
#define BYPASS_LAN_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct bypass_lan_plugin_t bypass_lan_plugin_t;

/**
 * Plugin installing bypass policies for locally attached subnets.
 */
struct bypass_lan_plugin_t {

	/**
	 * Implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** BYPASS_LAN_PLUGIN_H_ @}*/
