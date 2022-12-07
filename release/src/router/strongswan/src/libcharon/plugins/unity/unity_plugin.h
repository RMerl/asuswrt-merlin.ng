/*
 * Copyright (C) 2012 Martin Willi
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
 * @defgroup unity unity
 * @ingroup cplugins
 *
 * @defgroup unity_plugin unity_plugin
 * @{ @ingroup unity
 */

#ifndef UNITY_PLUGIN_H_
#define UNITY_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct unity_plugin_t unity_plugin_t;

/**
 * IKEv1 Cisco Unity extension support.
 */
struct unity_plugin_t {

	/**
	 * Implements plugin_t. interface.
	 */
	plugin_t plugin;
};

#endif /** UNITY_PLUGIN_H_ @}*/
