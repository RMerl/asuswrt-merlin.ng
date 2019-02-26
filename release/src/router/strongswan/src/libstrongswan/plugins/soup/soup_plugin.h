/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup soup_p soup
 * @ingroup plugins
 *
 * @defgroup soup_plugin soup_plugin
 * @{ @ingroup soup_p
 */

#ifndef SOUP_PLUGIN_H_
#define SOUP_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct soup_plugin_t soup_plugin_t;

/**
 * Plugin implementing fetcher interface for HTTP using libsoup.
 */
struct soup_plugin_t {

	/**
	 * Implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** SOUP_PLUGIN_H_ @}*/
