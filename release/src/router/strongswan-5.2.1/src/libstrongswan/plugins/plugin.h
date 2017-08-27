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
 * @defgroup plugin plugin
 * @{ @ingroup plugins
 */

#ifndef PLUGIN_H_
#define PLUGIN_H_

typedef struct plugin_t plugin_t;

#include <library.h>
#include <plugins/plugin_feature.h>

/**
 * Interface definition of a plugin.
 */
struct plugin_t {

	/**
	 * Get the name of the plugin.
	 *
	 * @return		plugin name
	 */
	char* (*get_name)(plugin_t *this);

	/**
	 * Get plugin features with dependencies.
	 *
	 * The returned array contains features provided by the plugin and
	 * dependencies for that feature. See plugin_feature_t for details.
	 *
	 * @param features	pointer receiving plugin features
	 * @return			number of features
	 */
	int (*get_features)(plugin_t *this, plugin_feature_t *features[]);

	/**
	 * Try to reload plugin configuration.
	 *
	 * @return		TRUE if reloaded, FALSE if reloading not supporty by plugin
	 */
	bool (*reload)(plugin_t *this);

	/**
	 * Destroy a plugin instance.
	 */
	void (*destroy)(plugin_t *this);
};


/**
 * Plugin constructor function definition.
 *
 * Each plugin has a constructor function. This function is called on daemon
 * startup to initialize each plugin.
 * The plugin function is named plugin_name_plugin_create().
 *
 * @return				plugin_t instance
 */
typedef plugin_t *(*plugin_constructor_t)(void);

#endif /** PLUGIN_H_ @}*/
