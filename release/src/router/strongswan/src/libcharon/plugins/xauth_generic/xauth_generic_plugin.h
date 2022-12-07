/*
 * Copyright (C) 2011 Tobias Brunner
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
 * @defgroup xauth_generic xauth_generic
 * @ingroup cplugins
 *
 * @defgroup xauth_generic_plugin xauth_generic_plugin
 * @{ @ingroup xauth_generic
 */

#ifndef XAUTH_GENERIC_PLUGIN_H_
#define XAUTH_GENERIC_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct xauth_generic_plugin_t xauth_generic_plugin_t;

/**
 * XAuth generic plugin using secrets defined in ipsec.secrets.
 */
struct xauth_generic_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** XAUTH_GENERIC_PLUGIN_H_ @}*/
