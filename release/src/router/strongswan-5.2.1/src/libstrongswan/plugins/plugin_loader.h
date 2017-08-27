/*
 * Copyright (C) 2012-2014 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup plugin_loader plugin_loader
 * @{ @ingroup plugins
 */

#ifndef PLUGIN_LOADER_H_
#define PLUGIN_LOADER_H_

typedef struct plugin_loader_t plugin_loader_t;

#include <collections/enumerator.h>
#include <utils/debug.h>

/* to avoid circular references we can't include plugin_feature.h */
struct plugin_feature_t;

/**
 * The plugin_loader loads plugins from a directory and initializes them
 */
struct plugin_loader_t {

	/**
	 * Add static plugin features, not loaded via plugins.
	 *
	 * Similar to features provided by plugins they are evaluated during load(),
	 * and unloaded when unload() is called.
	 *
	 * If critical is TRUE load() will fail if any of the added features could
	 * not be loaded.
	 *
	 * If a reload callback function is given, it gets invoked for the
	 * registered feature set when reload() is invoked on the plugin_loader.
	 *
	 * @note The name should be unique otherwise a plugin with the same name is
	 * not loaded.
	 *
	 * @param name			name of the component adding the features
	 * @param features		array of plugin features
	 * @param count			number of features in the array
	 * @param critical		TRUE if the features are critical
	 * @param reload		feature reload callback, or NULL
	 * @param reload_data	user data to pass to reload callback
	 */
	void (*add_static_features) (plugin_loader_t *this, const char *name,
								 struct plugin_feature_t *features, int count,
								 bool critical, bool (*reload)(void*),
								 void *reload_data);

	/**
	 * Load a list of plugins.
	 *
	 * Each plugin in list may have an ending exclamation mark (!) to mark it
	 * as a critical plugin. If loading a critical plugin fails, plugin loading
	 * is aborted and FALSE is returned.
	 *
	 * Additional paths can be added with add_path(), these will be searched
	 * for the plugins first, in the order they were added, then the default
	 * path follows.
	 *
	 * If \<ns>.load_modular is enabled (where \<ns> is lib->ns) the plugins to
	 * load are determined via a load option in their respective plugin config
	 * section e.g. \<ns>.plugins.\<plugin>.load = <priority|bool>.
	 * The oder is determined by the configured priority.  If two plugins have
	 * the same priority the order as seen in list is preserved.  Plugins not
	 * found in list are loaded first, in alphabetical order.
	 *
	 * @note Even though this method could be called multiple times this is
	 * currently not really supported in regards to plugin features and their
	 * dependencies (in particular soft dependencies).
	 *
	 * @param list			space separated list of plugins to load
	 * @return				TRUE if all critical plugins loaded successfully
	 */
	bool (*load)(plugin_loader_t *this, char *list);

	/**
	 * Add an additional search path for plugins.
	 *
	 * These will be searched in the order they were added.
	 *
	 * @param path			path containing loadable plugins
	 */
	void (*add_path)(plugin_loader_t *this, char *path);

	/**
	 * Reload the configuration of one or multiple plugins.
	 *
	 * @param				space separated plugin names to reload, NULL for all
	 * @return				number of plugins that did support reloading
	 */
	u_int (*reload)(plugin_loader_t *this, char *list);

	/**
	 * Unload all loaded plugins.
	 */
	void (*unload)(plugin_loader_t *this);

	/**
	 * Create an enumerator over all loaded plugins.
	 *
	 * In addition to the plugin, the enumerator optionally provides a list of
	 * pointers to plugin features currently loaded.
	 * This list has to be destroyed.
	 *
	 * @return				enumerator over plugin_t*, linked_list_t*
	 */
	enumerator_t* (*create_plugin_enumerator)(plugin_loader_t *this);

	/**
	 * Check if the given feature is available and loaded.
	 *
	 * @param feature		feature to check
	 * @return				TRUE if feature available
	 */
	bool (*has_feature)(plugin_loader_t *this, struct plugin_feature_t feature);

	/**
	 * Get a simple list the names of all loaded plugins.
	 *
	 * The function returns internal data, do not free.
	 *
	 * @return				list of the names of all loaded plugins
	 */
	char* (*loaded_plugins)(plugin_loader_t *this);

	/**
	 * Log status about loaded plugins and features.
	 *
	 * @param level			log level to use
	 */
	void (*status)(plugin_loader_t *this, level_t level);

	/**
	 * Unload loaded plugins, destroy plugin_loader instance.
	 */
	void (*destroy)(plugin_loader_t *this);
};

/**
 * Create a plugin_loader instance.
 *
 * @return			plugin loader instance
 */
plugin_loader_t *plugin_loader_create();

/**
 * Convenience function to add plugin directories for the given plugins within
 * the given base directory according to the conventions in the src/build tree.
 *
 * @param basedir	base directory
 * @param plugins	space separated list of plugins
 */
void plugin_loader_add_plugindirs(char *basedir, char *plugins);

#endif /** PLUGIN_LOADER_H_ @}*/
