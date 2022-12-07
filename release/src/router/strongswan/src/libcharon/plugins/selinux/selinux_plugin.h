/*
 * Copyright (C) 2022 Tobias Brunner
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
 * @defgroup selinux selinux
 * @ingroup cplugins
 *
 * @defgroup selinux_plugin selinux_plugin
 * @{ @ingroup selinux
 */

#ifndef SELINUX_PLUGIN_H_
#define SELINUX_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct selinux_plugin_t selinux_plugin_t;

/**
 * Plugin managing trap policies with generic SELinux labels.
 */
struct selinux_plugin_t {

	/**
	 * Implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** SELINUX_PLUGIN_H_ @}*/
