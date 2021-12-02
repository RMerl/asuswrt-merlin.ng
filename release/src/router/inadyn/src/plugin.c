/* Simplistic plugin support for DDNS service providers
 *
 * Copyright (c) 2012-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "config.h"
#include <assert.h>
#include <errno.h>
#include <dirent.h>		/* readdir() et al */
#include <string.h>

#include "ddns.h"

static char *plugpath = NULL;   /* Set by first load. */
static TAILQ_HEAD(, ddns_system) plugins = TAILQ_HEAD_INITIALIZER(plugins);

int plugin_register(ddns_system_t *plugin)
{
	if (!plugin) {
		errno = EINVAL;
		return 1;
	}

	assert(plugin->name);

	/* Already registered? */
	if (plugin_find(plugin->name, 0)) {
		logit(LOG_DEBUG, "... %s already loaded.", plugin->name);
		return 0;
	}

	TAILQ_INSERT_TAIL(&plugins, plugin, link);

	return 0;
}

int plugin_unregister(ddns_system_t *plugin)
{
	TAILQ_REMOVE(&plugins, plugin, link);

	/* XXX: Unfinished, add cleanup code here! */

	return 0;
}

static ddns_system_t *search_plugin(const char *name, int loose)
{
	ddns_system_t *p, *tmp;

	if (loose) {
		PLUGIN_ITERATOR(p, tmp) {
			if (strcasestr(p->name, name))
				return p;
		}

		return NULL;
	}

	PLUGIN_ITERATOR(p, tmp) {
		if (!strcasecmp(p->name, name))
			return p;
	}

	return NULL;
}


/**
 * plugin_find - Find a plugin by name
 * @name: With or without path, or .so extension
 * @loose: Use substring match when looking for plugin
 *
 * This function uses an opporunistic search for a suitable plugin and
 * returns the first match.  Albeit with at least some measure of
 * heuristics.
 *
 * First it checks for an exact match.  If no match is found and @name
 * starts with a slash the search ends.  Otherwise a new search with the
 * plugin path prepended to @name is made.  Also, if @name does not end
 * with .so it too is added to @name before searching.
 *
 * Returns:
 * On success the pointer to the matching &ddns_system_t is returned,
 * otherwise %NULL is returned.
 */
ddns_system_t *plugin_find(const char *name, int loose)
{
	char *tmp, *ptr;
	ddns_system_t *p;

	if (!name) {
		errno = EINVAL;
		return NULL;
	}

	/* Check for multiple instances of plugin */
	tmp = strdup(name);
	if (!tmp)
		return NULL;

	ptr = strchr(tmp, ':');
	if (ptr)
		*ptr = 0;
	name = tmp;

	p = search_plugin(name, loose);
	if (p) {
		free(tmp);
		return p;
	}

	if (plugpath && name[0] != '/') {
		int noext;
		char *path;
		size_t len = strlen(plugpath) + strlen(name) + 5;

		path = malloc(len);
		if (!path) {
			free(tmp);
			return NULL;
		}

		noext = strcmp(name + strlen(name) - 3, ".so");
		snprintf(path, len, "%s%s%s%s", plugpath,
			 plugpath[strlen(plugpath) - 1] == '/' ? "" : "/",
			 name, noext ? ".so" : "");

		p = search_plugin(path, loose);
		free(path);
		if (p) {
			free(tmp);
			return p;
		}
	}

	free(tmp);
	errno = ENOENT;

	return NULL;
}

/* Private daemon API *******************************************************/
#if o
/**
 * load_one - Load one plugin
 * @path: Path to finit plugins, usually %PLUGIN_PATH
 * @name: Name of plugin, optionally ending in ".so"
 *
 * Loads a plugin from @path/@name[.so].  Note, if ".so" is missing from
 * the plugin @name it is added before attempting to load.
 *
 * It is up to the plugin itself ot register itself as a "ctor" with the
 * %PLUGIN_INIT macro so that plugin_register() is called automatically.
 *
 * Returns:
 * POSIX OK(0) on success, non-zero otherwise.
 */
static int load_one(char *path, char *name)
{
	int noext;
	char plugin[256];
	void *handle;

	if (!path || !name) {
		errno = EINVAL;
		return 1;
	}

	/* Compose full path, with optional .so extension, to plugin */
	noext = strcmp(name + strlen(name) - 3, ".so");
	snprintf(plugin, sizeof(plugin), "%s/%s%s", path, name, noext ? ".so" : "");

	logit(LOG_DEBUG, "Loading plugin %s ...", basename(plugin));
	handle = dlopen(plugin, RTLD_LAZY | RTLD_GLOBAL);
	if (!handle) {
		logit(LOG_ERR, "Failed loading plugin %s: %s", plugin, dlerror());
		return 1;
	}

	return 0;
}

int plugin_load_all(char *path)
{
	int fail = 0;
	DIR *dp = opendir(path);
	struct dirent *entry;

	if (!dp) {
		logit(LOG_ERR, "Failed, cannot open plugin directory %s: %s", path, strerror(errno));
		return 1;
	}
	plugpath = path;

	while ((entry = readdir(dp))) {
		if (entry->d_name[0] == '.')
			continue; /* Skip . and .. directories */

		if (load_one(path, entry->d_name))
			fail++;
	}

	closedir(dp);

	return fail;
}
#endif

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
