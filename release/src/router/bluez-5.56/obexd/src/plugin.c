// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/stat.h>

#include <glib.h>

#include "obexd.h"
#include "plugin.h"
#include "log.h"

/*
 * Plugins that are using libraries with threads and their own mainloop
 * will crash on exit. This is a bug inside these libraries, but there is
 * nothing much that can be done about it. One bad example is libebook.
 */
#ifdef NEED_THREADS
#define PLUGINFLAG (RTLD_NOW | RTLD_NODELETE)
#else
#define PLUGINFLAG (RTLD_NOW)
#endif

static GSList *plugins = NULL;

struct obex_plugin {
	void *handle;
	struct obex_plugin_desc *desc;
};

static gboolean add_plugin(void *handle, struct obex_plugin_desc *desc)
{
	struct obex_plugin *plugin;

	if (desc->init == NULL)
		return FALSE;

	plugin = g_try_new0(struct obex_plugin, 1);
	if (plugin == NULL)
		return FALSE;

	plugin->handle = handle;
	plugin->desc = desc;

	if (desc->init() < 0) {
		g_free(plugin);
		return FALSE;
	}

	plugins = g_slist_append(plugins, plugin);
	DBG("Plugin %s loaded", desc->name);

	return TRUE;
}

static gboolean check_plugin(struct obex_plugin_desc *desc,
				char **patterns, char **excludes)
{
	if (excludes) {
		for (; *excludes; excludes++)
			if (g_pattern_match_simple(*excludes, desc->name))
				break;
		if (*excludes) {
			info("Excluding %s", desc->name);
			return FALSE;
		}
	}

	if (patterns) {
		for (; *patterns; patterns++)
			if (g_pattern_match_simple(*patterns, desc->name))
				break;
		if (*patterns == NULL) {
			info("Ignoring %s", desc->name);
			return FALSE;
		}
	}

	return TRUE;
}


#include "builtin.h"

gboolean plugin_init(const char *pattern, const char *exclude)
{
	char **patterns = NULL;
	char **excludes = NULL;
	GDir *dir;
	const char *file;
	unsigned int i;

	if (strlen(PLUGINDIR) == 0)
		return FALSE;

	if (pattern)
		patterns = g_strsplit_set(pattern, ":, ", -1);

	if (exclude)
		excludes = g_strsplit_set(exclude, ":, ", -1);

	DBG("Loading builtin plugins");

	for (i = 0; __obex_builtin[i]; i++) {
		if (check_plugin(__obex_builtin[i],
					patterns, excludes) == FALSE)
			continue;

		add_plugin(NULL,  __obex_builtin[i]);
	}

	DBG("Loading plugins %s", PLUGINDIR);

	dir = g_dir_open(PLUGINDIR, 0, NULL);
	if (!dir) {
		g_strfreev(patterns);
		g_strfreev(excludes);
		return FALSE;
	}

	while ((file = g_dir_read_name(dir)) != NULL) {
		struct obex_plugin_desc *desc;
		void *handle;
		char *filename;

		if (g_str_has_prefix(file, "lib") == TRUE ||
				g_str_has_suffix(file, ".so") == FALSE)
			continue;

		filename = g_build_filename(PLUGINDIR, file, NULL);

		handle = dlopen(filename, PLUGINFLAG);
		if (handle == NULL) {
			error("Can't load plugin %s: %s", filename,
								dlerror());
			g_free(filename);
			continue;
		}

		g_free(filename);

		desc = dlsym(handle, "obex_plugin_desc");
		if (desc == NULL) {
			error("Can't load plugin description: %s", dlerror());
			dlclose(handle);
			continue;
		}

		if (check_plugin(desc, patterns, excludes) == FALSE) {
			dlclose(handle);
			continue;
		}

		if (add_plugin(handle, desc) == FALSE)
			dlclose(handle);
	}

	g_dir_close(dir);
	g_strfreev(patterns);
	g_strfreev(excludes);

	return TRUE;
}

void plugin_cleanup(void)
{
	GSList *list;

	DBG("Cleanup plugins");

	for (list = plugins; list; list = list->next) {
		struct obex_plugin *plugin = list->data;

		if (plugin->desc->exit)
			plugin->desc->exit();

		if (plugin->handle != NULL)
			dlclose(plugin->handle);

		g_free(plugin);
	}

	g_slist_free(plugins);
}
