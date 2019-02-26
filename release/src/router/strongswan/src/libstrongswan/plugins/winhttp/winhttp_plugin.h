/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup winhttp_p winhttp
 * @ingroup plugins
 *
 * @defgroup winhttp_plugin winhttp_plugin
 * @{ @ingroup winhttp_p
 */

#ifndef WINHTTP_PLUGIN_H_
#define WINHTTP_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct winhttp_plugin_t winhttp_plugin_t;

/**
 * Plugin implementing fetcher interface using Microsofts WinHTTP.
 */
struct winhttp_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** WINHTTP_PLUGIN_H_ @}*/
