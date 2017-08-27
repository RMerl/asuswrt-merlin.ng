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
 * @defgroup curl_p curl
 * @ingroup plugins
 *
 * @defgroup curl_plugin curl_plugin
 * @{ @ingroup curl_p
 */

#ifndef CURL_PLUGIN_H_
#define CURL_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct curl_plugin_t curl_plugin_t;

/**
 * Plugin implementing fetcher interface using libcurl http library.
 */
struct curl_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** CURL_PLUGIN_H_ @}*/
